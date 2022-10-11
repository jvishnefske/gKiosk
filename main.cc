#include <memory>
#include <webkit2/webkit2.h>
#include <gtk/gtk.h>
#include <unistd.h>

extern "C"{
void activate(GtkApplication *app, gpointer user_data)
{
    (void) user_data;
    const auto char_url = getenv("KIOSK_URL");
    std::string url;
    if(char_url == nullptr){
        url = "https://example.com/";
    }else{
        url = char_url;
    }
//    std::shared_ptr<WebKitWebView> p_web_view{WEBKIT_WEB_VIEW(webkit_web_view_new()), g_object_unref};
    WebKitWebView *web_view = WEBKIT_WEB_VIEW(webkit_web_view_new());
//    std::shared_ptr<WebKitSettings> p_settings{webkit_settings_new(), g_object_unref};
    WebKitSettings *settings = webkit_settings_new();
    webkit_settings_set_enable_javascript(settings, true);
    webkit_settings_set_enable_java(settings, false);
    webkit_settings_set_enable_caret_browsing(settings, false);
    webkit_settings_set_enable_developer_extras(settings, true);
    webkit_settings_set_enable_fullscreen(settings, true);
    webkit_web_view_set_settings(web_view, settings);
    webkit_web_view_load_uri(web_view, url.c_str());
    GtkWindow *window{GTK_WINDOW(gtk_application_window_new(app))};
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);
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
