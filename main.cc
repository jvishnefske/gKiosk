#include <memory>
#include <webkit2/webkit2.h>
#include <gtk/gtk.h>
#include <cstdlib>
#include <string>
#include <algorithm>

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
    WebKitWebView *web_view = WEBKIT_WEB_VIEW(webkit_web_view_new());
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
}
}

int main(int argc, char** argv){
    std::shared_ptr<GtkApplication> p_app{gtk_application_new(nullptr, G_APPLICATION_FLAGS_NONE), g_object_unref};
    g_signal_connect(p_app.get(), "activate", G_CALLBACK(activate), nullptr);
    const auto status = g_application_run (G_APPLICATION (p_app.get()), argc, argv);
    return status;
}
