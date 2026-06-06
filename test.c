  #include <gtk/gtk.h>
  #include <mpd/client.h>
  #include <stdlib.h>
  #include <stdio.h>

  struct mpd_connection *mpd_conn = NULL;
  GtkTextBuffer *buffer = NULL;
  





  static void errorchecker(struct mpd_connection *conn) {
      if (mpd_connection_get_error(conn) != MPD_ERROR_SUCCESS) {
          fprintf(stderr, "error: %s\n", mpd_connection_get_error_message(conn));
      }
  }


  static void songdetection(void) {
      if (mpd_conn == NULL) return;

      struct mpd_song *song = mpd_run_current_song(mpd_conn);

      if (song != NULL) {
          const char *title = mpd_song_get_tag(song, MPD_TAG_TITLE, 0);
          const char *artist = mpd_song_get_tag(song, MPD_TAG_ARTIST, 0);
          if (title != NULL) {
              gtk_text_buffer_set_text(buffer, "currently playing: ", -1);
              gtk_text_buffer_insert_at_cursor(buffer, title, -1);
              gtk_text_buffer_insert_at_cursor(buffer, ", by ", -1);
              gtk_text_buffer_insert_at_cursor(buffer, artist, -1);
          } else {
              gtk_text_buffer_set_text(buffer, mpd_song_get_uri(song), -1);
          }
          mpd_song_free(song);
      } 
  }

  static void playButtonClicked(GtkWidget *widget, gpointer data) {
      if (mpd_conn) {
          mpd_run_play(mpd_conn);
          errorchecker(mpd_conn);
      }
  }


  static void nextButtonClicked(GtkWidget *widget, gpointer data) {
      if (mpd_conn) {
          mpd_run_next(mpd_conn);
          errorchecker(mpd_conn);
          songdetection();
      }
  }



  static void pauseButtonClicked(GtkWidget *widget, gpointer data) {
      if (mpd_conn) {
          mpd_run_toggle_pause(mpd_conn);
          errorchecker(mpd_conn);
      }
  }

  static void stopButtonClicked(GtkWidget *widget, gpointer data) {
      if (mpd_conn) {
          mpd_run_stop(mpd_conn);
          errorchecker(mpd_conn);
      }
  }

  static void on_destroy(GtkWidget *widget, gpointer data) {
      if (mpd_conn) {
          mpd_connection_free(mpd_conn);
      }
      gtk_main_quit();
  }





  int main(int argc, char *argv[]) {
      unsigned int port = 6600;

      if (argc > 1) {
          int parsed_port = atoi(argv[1]);

          port = (unsigned int)parsed_port;

      }

      gtk_init(&argc, &argv);

      mpd_conn = mpd_connection_new("localhost", port, 30000);
      if (mpd_conn == NULL) {
          fprintf(stderr, "out of memory\n");
          return 1;
      }
      if (mpd_connection_get_error(mpd_conn) != MPD_ERROR_SUCCESS) {
          fprintf(stderr, "couldnt connect to mpd on port %u: %s\n", 
                  port, mpd_connection_get_error_message(mpd_conn));
          mpd_connection_free(mpd_conn);
          return 1;
      }

      

      printf("connected on localhost:%u\n", port);

      GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
      gtk_window_set_title(GTK_WINDOW(window), "Noteblock");

      gtk_window_set_default_size(GTK_WINDOW(window), 300, 100);
      g_signal_connect(window, "destroy", G_CALLBACK(on_destroy), NULL);

      GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
      gtk_container_add(GTK_CONTAINER(window), main_box);

      GtkWidget *button_box = gtk_button_box_new(GTK_ORIENTATION_HORIZONTAL);
      gtk_box_pack_start(GTK_BOX(main_box), button_box, FALSE, FALSE, 5);

      GtkWidget *text_songinfo = gtk_text_view_new();
      gtk_text_view_set_editable(GTK_TEXT_VIEW(text_songinfo), FALSE);
    
      buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_songinfo));

      gtk_box_pack_start(GTK_BOX(main_box), text_songinfo, TRUE, TRUE, 5);

      buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_songinfo));

      gtk_container_add(GTK_CONTAINER(button_box), text_songinfo);

      GtkWidget *btn_play = gtk_button_new_with_label("Play");
      GtkWidget *btn_pause = gtk_button_new_with_label("Pause");
      GtkWidget *btn_stop = gtk_button_new_with_label("Stop");
      GtkWidget *btn_next = gtk_button_new_with_label("Next");

      gtk_container_add(GTK_CONTAINER(button_box), btn_play);
      gtk_container_add(GTK_CONTAINER(button_box), btn_pause);
      gtk_container_add(GTK_CONTAINER(button_box), btn_stop);
      gtk_container_add(GTK_CONTAINER(button_box), btn_next);

      g_signal_connect(btn_play, "clicked", G_CALLBACK(playButtonClicked), NULL);
      g_signal_connect(btn_pause, "clicked", G_CALLBACK(pauseButtonClicked), NULL);
      g_signal_connect(btn_stop, "clicked", G_CALLBACK(stopButtonClicked), NULL);
      g_signal_connect(btn_next, "clicked", G_CALLBACK(nextButtonClicked), NULL);

      songdetection();

      gtk_widget_show_all(window);
      gtk_main();



      return 0;
  }