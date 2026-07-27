#include <memory>
#include <webkit2/webkit2.h>
#include <gtk/gtk.h>
#include <cstdlib>
#include <string>
#include <algorithm>
#include <linux/joystick.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <cstring>
#include <cerrno>

static int js_fd = -1;
static WebKitWebView *g_web_view = nullptr;

static void setup_persistent_cookies(WebKitWebContext *context)
{
    WebKitCookieManager *cookie_manager = webkit_web_context_get_cookie_manager(context);
    std::string cookie_dir{[](){
        const auto *xdg = std::getenv("XDG_DATA_HOME");
        if (xdg != nullptr) {
            return std::string(xdg) + "/gkiosk";
        }
        const auto *home = std::getenv("HOME");
        if (home != nullptr) {
            return std::string(home) + "/.local/share/gkiosk";
        }
        return std::string("/tmp/gkiosk");
    }()};
    g_mkdir_with_parents(cookie_dir.c_str(), 0700);
    std::string cookie_file = cookie_dir + "/cookies.sqlite";
    webkit_cookie_manager_set_persistent_storage(
        cookie_manager,
        cookie_file.c_str(),
        WEBKIT_COOKIE_PERSISTENT_STORAGE_SQLITE
    );
    webkit_cookie_manager_set_accept_policy(
        cookie_manager,
        WEBKIT_COOKIE_POLICY_ACCEPT_NO_THIRD_PARTY
    );
}

static int open_joystick()
{
    DIR *dir = opendir("/dev/input");
    if (dir == nullptr) {
        return -1;
    }
    struct dirent *entry;
    while ((entry = readdir(dir)) != nullptr) {
        if (std::strncmp(entry->d_name, "js", 2) == 0) {
            std::string path = std::string("/dev/input/") + entry->d_name;
            int fd = open(path.c_str(), O_RDONLY | O_NONBLOCK);
            if (fd >= 0) {
                closedir(dir);
                return fd;
            }
        }
    }
    closedir(dir);
    return -1;
}

static void inject_key_event(WebKitWebView *web_view, guint keyval)
{
    GtkWidget *widget = GTK_WIDGET(web_view);
    GdkWindow *gdk_window = gtk_widget_get_window(widget);
    if (gdk_window == nullptr) {
        return;
    }
    GdkEvent *press = gdk_event_new(GDK_KEY_PRESS);
    press->key.window = static_cast<GdkWindow*>(g_object_ref(gdk_window));
    press->key.keyval = keyval;
    press->key.send_event = TRUE;
    press->key.time = GDK_CURRENT_TIME;
    gtk_main_do_event(press);

    GdkEvent *release = gdk_event_new(GDK_KEY_RELEASE);
    release->key.window = static_cast<GdkWindow*>(g_object_ref(gdk_window));
    release->key.keyval = keyval;
    release->key.send_event = TRUE;
    release->key.time = GDK_CURRENT_TIME;
    gtk_main_do_event(release);

    gdk_event_free(press);
    gdk_event_free(release);
}

static gboolean poll_joystick(gpointer user_data)
{
    (void)user_data;
    static int prev_axis_dir[8] = {0};
    if (js_fd < 0 || g_web_view == nullptr) {
        js_fd = open_joystick();
        return TRUE;
    }
    struct js_event event;
    while (read(js_fd, &event, sizeof(event)) == sizeof(event)) {
        if (event.type == JS_EVENT_BUTTON && event.value == 1) {
            switch (event.number) {
                case 0: // A button - Enter/Select
                    inject_key_event(g_web_view, GDK_KEY_Return);
                    break;
                case 1: // B button - Back
                    webkit_web_view_go_back(g_web_view);
                    break;
                case 6: // Select - go to home URL
                    {
                        const auto *char_url = std::getenv("KIOSK_URL");
                        std::string url = char_url ? char_url : "https://example.com/";
                        webkit_web_view_load_uri(g_web_view, url.c_str());
                    }
                    break;
                default:
                    break;
            }
        } else if (event.type == JS_EVENT_AXIS) {
            const int threshold = 16384;
            int dir = 0;
            guint keyval_neg = 0, keyval_pos = 0;
            switch (event.number) {
                case 0: // Left stick X-axis
                    if (event.value < -threshold) dir = -1;
                    else if (event.value > threshold) dir = 1;
                    keyval_neg = GDK_KEY_Left;
                    keyval_pos = GDK_KEY_Right;
                    break;
                case 1: // Left stick Y-axis
                    if (event.value < -threshold) dir = -1;
                    else if (event.value > threshold) dir = 1;
                    keyval_neg = GDK_KEY_Up;
                    keyval_pos = GDK_KEY_Down;
                    break;
                case 6: // D-pad X-axis
                    if (event.value < 0) dir = -1;
                    else if (event.value > 0) dir = 1;
                    keyval_neg = GDK_KEY_Left;
                    keyval_pos = GDK_KEY_Right;
                    break;
                case 7: // D-pad Y-axis
                    if (event.value < 0) dir = -1;
                    else if (event.value > 0) dir = 1;
                    keyval_neg = GDK_KEY_Up;
                    keyval_pos = GDK_KEY_Down;
                    break;
                default:
                    break;
            }
            if (event.number <= 7 && dir != prev_axis_dir[event.number]) {
                if (dir == -1) {
                    inject_key_event(g_web_view, keyval_neg);
                } else if (dir == 1) {
                    inject_key_event(g_web_view, keyval_pos);
                }
                prev_axis_dir[event.number] = dir;
            }
        }
    }
    if (errno != EAGAIN && errno != EWOULDBLOCK) {
        close(js_fd);
        js_fd = -1;
        std::memset(prev_axis_dir, 0, sizeof(prev_axis_dir));
    }
    return TRUE;
}

extern "C"{
void activate(GtkApplication *app, gpointer user_data)
{
    (void) user_data;
    const auto url{[](){
        const auto * char_url = std::getenv("KIOSK_URL");
        if(char_url == nullptr){
            return std::string("https://example.com/");
        }
        return std::string(char_url);
    }()};

    WebKitWebContext *context = webkit_web_context_get_default();
    setup_persistent_cookies(context);

    WebKitWebView *web_view = WEBKIT_WEB_VIEW(webkit_web_view_new_with_context(context));
    g_web_view = web_view;
    WebKitSettings *settings = webkit_settings_new();
    webkit_settings_set_enable_javascript(settings, gtk_true());
    webkit_settings_set_enable_java(settings, gtk_false());
    webkit_settings_set_enable_caret_browsing(settings, gtk_false());
    webkit_settings_set_enable_developer_extras(settings, gtk_true());
    webkit_settings_set_enable_fullscreen(settings, gtk_true());
    webkit_web_view_set_settings(web_view, settings);
    webkit_web_view_load_uri(web_view, url.c_str());
    GtkWindow *window{GTK_WINDOW(gtk_application_window_new(app))};
    g_object_unref(settings);
    gtk_window_fullscreen(GTK_WINDOW(window));
    gtk_container_add(GTK_CONTAINER(window), GTK_WIDGET(web_view));
    gtk_widget_show_all(GTK_WIDGET(window));

    js_fd = open_joystick();
    g_timeout_add(16, poll_joystick, nullptr);
}
}

int main(int argc, char** argv){
    std::shared_ptr<GtkApplication> p_app{gtk_application_new(nullptr, G_APPLICATION_FLAGS_NONE), g_object_unref};
    g_signal_connect(p_app.get(), "activate", G_CALLBACK(activate), nullptr);
    const auto status = g_application_run (G_APPLICATION (p_app.get()), argc, argv);
    if (js_fd >= 0) {
        close(js_fd);
    }
    return status;
}
