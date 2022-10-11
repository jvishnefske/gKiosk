#include <memory>
#include <iostream>
#include <webkit2/webkit2.h>
#include <gtk/gtk.h>

extern "C"{
void activate(GtkApplication * app, gpointer user_data){
    (void) user_data;
  GtkWidget * window;
  window = gtk_application_window_new (app);
  gtk_window_set_title (GTK_WINDOW (window), "Window");
  gtk_window_set_default_size (GTK_WINDOW (window), 200, 200);

  gtk_widget_show_all (window);
}
} // extern

int main(int argc, char** argv){
    std::shared_ptr<GtkApplication> p_app{gtk_application_new(nullptr, G_APPLICATION_FLAGS_NONE), g_object_unref};
    g_signal_connect(p_app.get(), "activate", G_CALLBACK(activate), nullptr);
    const auto status = g_application_run (G_APPLICATION (p_app.get()), argc, argv);
    return status;
}
