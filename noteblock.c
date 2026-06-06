  #include <gtk/gtk.h>
  #include <mpd/client.h>
  #include <stdlib.h>
  #include <stdio.h>

  struct mpd_connection *mpd_conn = NULL;
  GtkTextBuffer *buffer = NULL;
  GtkWidget *progress = NULL;


  static void errorchecker(struct mpd_connection *conn) {
      if (mpd_connection_get_error(conn) != MPD_ERROR_SUCCESS) {
          fprintf(stderr, "error: %s\n", mpd_connection_get_error_message(conn));
      }
  }


  static gboolean songdetection(gpointer data) {
      if (mpd_conn == NULL || buffer == NULL) return TRUE;

      struct mpd_status *status = mpd_run_status(mpd_conn);
        if (status != NULL) {
        unsigned elapsed = mpd_status_get_elapsed_time(status);
        unsigned total = mpd_status_get_total_time(status);

        if (total > 0) {
            // im bad at mathhhhhh
            double fraction = (double)elapsed / (double)total;
            gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progress), fraction);
        } else {
            gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progress), 0.0);
        }
        mpd_status_free(status);
    }

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
      return TRUE;
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
      }
  }

    static void previousButtonClicked(GtkWidget *widget, gpointer data) {
      if (mpd_conn) {
          mpd_run_previous(mpd_conn);
          errorchecker(mpd_conn);
      }
  }

    static void add5secButtonClicked(GtkWidget *widget, gpointer data) {
      if (mpd_conn) {
          struct mpd_status *status = mpd_run_status(mpd_conn);
          if (status != NULL) {
              int current_pos = mpd_status_get_song_pos(status);
              unsigned elapsed = mpd_status_get_elapsed_time(status);
              unsigned new_time = elapsed + 5;
              mpd_run_seek_pos(mpd_conn, current_pos, new_time);
              mpd_status_free(status);
          }
          errorchecker(mpd_conn);
      }
  }

  static void remove5secButtonClicked(GtkWidget *widget, gpointer data) {
      if (mpd_conn) {
          struct mpd_status *status = mpd_run_status(mpd_conn);
          if (status != NULL) {
              int current_pos = mpd_status_get_song_pos(status);
              unsigned elapsed = mpd_status_get_elapsed_time(status);
              unsigned new_time = elapsed - 5;
              mpd_run_seek_pos(mpd_conn, current_pos, new_time);
              mpd_status_free(status);
          }
          errorchecker(mpd_conn);
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

      GtkWidget *button_box2 = gtk_button_box_new(GTK_ORIENTATION_HORIZONTAL);
      gtk_box_pack_start(GTK_BOX(main_box), button_box2, FALSE, FALSE, 5);
      gtk_button_box_set_layout(GTK_BUTTON_BOX(button_box2), GTK_BUTTONBOX_CENTER);

      GtkWidget *text_songinfo = gtk_text_view_new();
      gtk_text_view_set_editable(GTK_TEXT_VIEW(text_songinfo), FALSE);

      buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_songinfo));
     
      gtk_box_pack_start(GTK_BOX(main_box), text_songinfo, TRUE, TRUE, 5);

      buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text_songinfo));

      gtk_container_add(GTK_CONTAINER(button_box), text_songinfo);

      progress = gtk_progress_bar_new();

      gtk_box_pack_start(GTK_BOX(main_box), progress, FALSE, FALSE, 5);

      GtkWidget *btn_play = gtk_button_new_with_label("Play");
      GtkWidget *btn_pause = gtk_button_new_with_label("Pause");
      GtkWidget *btn_stop = gtk_button_new_with_label("Stop");
      GtkWidget *btn_next = gtk_button_new_with_label("Next");
      GtkWidget *btn_previous = gtk_button_new_with_label("Previous");
      GtkWidget *btn_add5sec = gtk_button_new_with_label("+5s");
      GtkWidget *btn_remove5sec = gtk_button_new_with_label("-5s");

      gtk_container_add(GTK_CONTAINER(button_box), btn_play);
      gtk_container_add(GTK_CONTAINER(button_box), btn_pause);
      gtk_container_add(GTK_CONTAINER(button_box), btn_stop);
      gtk_container_add(GTK_CONTAINER(button_box), btn_next);
      gtk_container_add(GTK_CONTAINER(button_box), btn_previous);
      gtk_container_add(GTK_CONTAINER(button_box2), btn_add5sec);
      gtk_container_add(GTK_CONTAINER(button_box2), btn_remove5sec);

      g_signal_connect(btn_play, "clicked", G_CALLBACK(playButtonClicked), NULL);
      g_signal_connect(btn_pause, "clicked", G_CALLBACK(pauseButtonClicked), NULL);
      g_signal_connect(btn_stop, "clicked", G_CALLBACK(stopButtonClicked), NULL);
      g_signal_connect(btn_next, "clicked", G_CALLBACK(nextButtonClicked), NULL);
      g_signal_connect(btn_previous, "clicked", G_CALLBACK(previousButtonClicked), NULL);
        g_signal_connect(btn_add5sec, "clicked", G_CALLBACK(add5secButtonClicked), NULL);
      g_signal_connect(btn_remove5sec, "clicked", G_CALLBACK(remove5secButtonClicked), NULL);

      g_timeout_add(1000, (GSourceFunc)songdetection, NULL);

      gtk_widget_show_all(window);
      gtk_main();



      return 0;
  }
