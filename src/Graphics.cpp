/*
 * Graphics.cpp
 *
 *  Created on: Aug 13, 2011
 *      Author: ben
 */

#include "Graphics.h"

//while (gtk_events_pending ()) gtk_main_iteration();
//gdk_window_process_all_updates();
//gtk_widget_queue_draw(GW("ClientSendText"));

Graphics::Graphics() {
	// TODO Auto-generated constructor stub
	cout << "Constructing Graphics...\n";
	Message = new char();
	LogFD = new FILE();
	LogInit();
}

Graphics::~Graphics() {
	// TODO Auto-generated destructor stub
	cout << "Destructing Graphics...\n";
}

GObject* Graphics::GO(const char* objectname) {
	return (gtk_builder_get_object(Builder, objectname));
}
GtkWidget* Graphics::GW(const char* widgetname) {
	return GTK_WIDGET(gtk_builder_get_object(Builder, widgetname));
}

void Graphics::BufferInsert(const char *tag) {
	if (!strcmp(ProgramMode, "Server")) {
		GtkTextIter EndIterMain;
		GtkTextMark *mark;
		ServerTextBuffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(GO(
				"ServerTextView")));
		EIter(ServerTextBuffer,EndIterMain);
		gtk_text_buffer_insert_with_tags_by_name(ServerTextBuffer,
				&EndIterMain, GetCurrentTime().c_str(), -1, "time", NULL);
		gtk_text_buffer_insert_with_tags_by_name(ServerTextBuffer,
				&EndIterMain, Message, -1, tag, NULL);
		mark = gtk_text_buffer_create_mark(ServerTextBuffer, "end",
				&EndIterMain, FALSE);
		gtk_text_view_scroll_to_mark(GTK_TEXT_VIEW(GO("ServerTextView")), mark,
				0.05, TRUE, 0.0, 1.0);
		gtk_widget_queue_draw(GW("ServerTextView"));
	} else if (!strcmp(ProgramMode, "Client")) {
		GtkTextIter EndIterMain;
		GtkTextMark *mark;
		ClientTextBuffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(GO(
				"ClientTextView")));
		EIter(ClientTextBuffer,EndIterMain);
		gtk_text_buffer_insert_with_tags_by_name(ClientTextBuffer,
				&EndIterMain, GetCurrentTime().c_str(), -1, "time", NULL);
		gtk_text_buffer_insert_with_tags_by_name(ClientTextBuffer,
				&EndIterMain, Message, -1, tag, NULL);
		mark = gtk_text_buffer_create_mark(ClientTextBuffer, "end",
				&EndIterMain, FALSE);
		gtk_text_view_scroll_to_mark(GTK_TEXT_VIEW(GO("ClientTextView")), mark,
				0.05, TRUE, 0.0, 1.0);
		gtk_widget_queue_draw(GW("ClientTextView"));
	}
}

char * Graphics::BufferGet() {
	GtkTextIter StartIterSend, EndIterSend;
	char sendtxt[225] = "";
	ClientTextBuffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(GO(
			"ClientTextView")));
	ClientSendBuffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(GO(
			"ClientSendText")));
	SIter(ClientSendBuffer,StartIterSend);
	EIter(ClientSendBuffer,EndIterSend);
	strncpy(sendtxt, (char*) gtk_text_iter_get_text(&StartIterSend,
			&EndIterSend), 180);
	gtk_text_buffer_set_text(ClientSendBuffer, "", 0);
	gdk_window_process_all_updates();
	usleep(100000);
	return sendtxt;
}

//Show Message****************************************************************
//Function to open a dialog box displaying the message provided.
void Graphics::quick_message(const char *message) {
	GtkWidget *dialog, *label, *content_area;
	/* Create the widgets */
	dialog = gtk_dialog_new_with_buttons("Message", GTK_WINDOW(GO(
			"ClientWindow")), GTK_DIALOG_DESTROY_WITH_PARENT, GTK_STOCK_OK,
			GTK_RESPONSE_NONE, NULL);
	content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
	label = gtk_label_new((gchar*) message);
	/* Ensure that the dialog box is destroyed when the user responds. */
	g_signal_connect_swapped(dialog, "response",
			G_CALLBACK(gtk_widget_destroy), dialog);
	/* Add the label, and show everything we’ve added to the dialog. */
	gtk_container_add(GTK_CONTAINER(content_area), label);
	gtk_widget_show_all(dialog);
}

void Graphics::ClientCheckUpdate(const char buf[256]) {
	int i = 0;
	while (i <= 9) {
		if (!strncmp(&buf[i + 10], "1", 1)) {
			stringstream x;
			char tmp[20];
			strncpy(tmp, &buf[20 * (i + 1)], 20);
			x << "ClientC";
			x << (i + 1);
			x << "\0";
			gtk_widget_set_sensitive(GW(x.str().c_str()), true);
			GTK_TOGGLE_BUTTON(GW(x.str().c_str()))->active = true;
			gtk_button_set_label(GTK_BUTTON(GW(x.str().c_str())), (gchar*) tmp);
		} else if (!strncmp(&buf[i + 10], "0", 1)) {
			stringstream x;
			x << "ClientC";
			x << (i + 1);
			x << "\0";
			GTK_TOGGLE_BUTTON(GW(x.str().c_str()))->active = false;
			gtk_button_set_label(GTK_BUTTON(GW(x.str().c_str())),
					"Available...");
			gtk_widget_set_sensitive(GW(x.str().c_str()), false);
		}
		i++;
		usleep(1000);
	}
}

void Graphics::ClientCheck(const int num, const char name[20]) {
	if (num > 0) {
		stringstream x;
		x << "ServerC";
		x << num;
		x << "\0";
		gtk_widget_set_sensitive(GW(x.str().c_str()), true);
		GTK_TOGGLE_BUTTON(GW(x.str().c_str()))->active = true;
		gtk_button_set_label(GTK_BUTTON(GW(x.str().c_str())), (gchar*) name);
	} else if (num < 0) {
		stringstream x;
		x << "ServerC";
		x << abs(num);
		x << "\0";
		GTK_TOGGLE_BUTTON(GW(x.str().c_str()))->active = false;
		gtk_button_set_label(GTK_BUTTON(GW(x.str().c_str())), "Available...");
		gtk_widget_set_sensitive(GW(x.str().c_str()), false);
	}
}

char* Graphics::getClientCheck() {
	int i = 0;
	char check[10];
	if (!strcmp(ProgramMode, "Server")) {
		while (i <= 9) {
			//gdk_threads_leave();
			stringstream x;
			x << "ServerC";
			x << (i + 1);
			x << "\0";
			if (gtk_widget_get_sensitive(GW(x.str().c_str()))) {
				if (GTK_TOGGLE_BUTTON(GW(x.str().c_str()))->active) {
					strcpy(&check[i], "1");
				}
			} else {
				strcpy(&check[i], "0");
			}
			i++;
			usleep(10000);
		}
	} else {
		while (i <= 9) {
			stringstream x;
			x << "ClientC";
			x << (i + 1);
			x << "\0";
			if (gtk_widget_get_sensitive(GW(x.str().c_str()))) {
				if (GTK_TOGGLE_BUTTON(GW(x.str().c_str()))->active) {
					strcpy(&check[i], "1");
				}
			} else {
				strcpy(&check[i], "0");
			}
			i++;
			usleep(10000);
		}
	}
	return check;
}

string Graphics::GetCurrentTime() {
	time_t CurTime = time(0);
	tm *localtm = localtime(&CurTime);
	char t[50] = "", tmp[50] = "";
	strcpy(tmp, asctime(localtm));
	t[0] = '[';
	int z = 0;
	while (tmp[z] != '\n') {
		t[z + 1] = tmp[z];
		z++;
	}
	t[z + 1] = ']';
	t[z + 2] = ' ';
	t[z + 3] = '\0';
	return t;
}

void Graphics::LogInit() {
	string t;
	system("mkdir -p Log");
	t.assign("./Log/Log ");
	t.append(GetCurrentTime());
	t[strlen(t.c_str()) - 1] = 0;
	LogFD = fopen(t.c_str(), "w");
}
