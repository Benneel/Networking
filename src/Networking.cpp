//============================================================================
// Name        : Networking.cpp
// Author      : Ben
// Version     :
// Copyright   : Freeware
// Description : Hello World in C++, Ansi-style
//============================================================================

#include "Communications.h"

char gProgramMode[10];
char gConnectionType[10];
char gIPversion[10];
char gIPAddress[15];
char gPort[10];
char gPassword[15];
bool gNonBlockingMode;
int gEncryptionType;

Communications tmp;

void* RunServerThread(void*);
void* RunClientThread(void*);
void* SendMessage(void*);

#define errexit(code,str)                          \
  fprintf(stderr,"%s: %s\n",(str),strerror(code));

//Program Mode****************************************************************
extern "C" void ConfigureAsServer(GtkWidget *widget, gpointer data) {
	strcpy(gProgramMode, "Server");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp.GW("TypeServerTCP")),
			true);
	gtk_widget_show(tmp.GW("TypeServerWindow"));
	gtk_widget_hide(tmp.GW("ModeWindow"));
	fprintf(tmp.LogFD, "%sStarting program in 'SERVER MODE'\n",
			tmp.GetCurrentTime().c_str());
}
extern "C" void ConfigureAsClient(GtkWidget *widget, gpointer data) {
	strcpy(gProgramMode, "Client");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tmp.GW("TypeClientTCP")),
			true);
	gtk_entry_set_text(GTK_ENTRY(tmp.GW("TypeClientUserName")),
			(gchar*) tmp.HostName);

	gtk_widget_show(tmp.GW("TypeClientWindow"));
	gtk_widget_hide(tmp.GW("ModeWindow"));
	fprintf(tmp.LogFD, "%sStarting program in 'CLIENT MODE'\n",
			tmp.GetCurrentTime().c_str());
}
//Program Mode****************************************************************
//----------------------------------------------------------------------------
//Configure Server************************************************************
extern "C" void ConfigureServer(GtkWidget *widget, gpointer data) {
	bool InfoCheck = true;
	char *markup;
	if (GTK_TOGGLE_BUTTON(tmp.GW("TypeServerTCP"))->active) {
		strcpy(gConnectionType, "TCP");
	} else if (GTK_TOGGLE_BUTTON(tmp.GW("TypeServerUDP"))->active) {
		strcpy(gConnectionType, "UDP");
	}
	fprintf(tmp.LogFD, "%sServer Connection Type: '%s'\n",
			tmp.GetCurrentTime().c_str(), gConnectionType);

	if (GTK_TOGGLE_BUTTON(tmp.GW("TypeServerIPv4"))->active) {
		strcpy(gIPversion, "IPv4");
	} else if (GTK_TOGGLE_BUTTON(tmp.GW("TypeServerIPv6"))->active) {
		strcpy(gIPversion, "IPv6");
	} else {
		strcpy(gIPversion, "UNSPEC");
	}
	fprintf(tmp.LogFD, "%sServer IP Version: '%s'\n",
			tmp.GetCurrentTime().c_str(), gIPversion);

	if (!strcmp(strcpy(gIPAddress,
			(char*) (GTK_ENTRY(tmp.GW("TypeServerIP")))->text), "")) {
		markup = g_markup_printf_escaped("<span foreground=\"red\">%s</span>",
				"IP:		");
		gtk_label_set_markup(GTK_LABEL(tmp.GW("TypeServerL4")), markup);
		g_free(markup);
		InfoCheck = false;
	} else {
		markup = g_markup_printf_escaped(
				"<span foreground=\"black\">%s</span>", "IP:		");
		gtk_label_set_markup(GTK_LABEL(tmp.GW("TypeServerL4")), markup);
		g_free(markup);
	}
	fprintf(tmp.LogFD, "%sServer IP Address: '%s'\n",
			tmp.GetCurrentTime().c_str(), gIPAddress);

	if (!strcmp(strcpy(gPort,
			(char*) (GTK_ENTRY(tmp.GW("TypeServerPort")))->text), "")) {
		markup = g_markup_printf_escaped("<span foreground=\"red\">%s</span>",
				"Port:	");
		gtk_label_set_markup(GTK_LABEL(tmp.GW("TypeServerL5")), markup);
		g_free(markup);
		InfoCheck = false;
	} else {
		markup = g_markup_printf_escaped(
				"<span foreground=\"black\">%s</span>", "Port:	");
		gtk_label_set_markup(GTK_LABEL(tmp.GW("TypeServerL5")), markup);
		g_free(markup);
	}
	fprintf(tmp.LogFD, "%sServer IP Port: '%s'\n",
			tmp.GetCurrentTime().c_str(), gPort);

	if (GTK_TOGGLE_BUTTON(tmp.GW("TypeServerNonBlock"))->active) {
		gNonBlockingMode = true;
		fprintf(tmp.LogFD, "%sServer with 'NonBlocking' sockets\n",
				tmp.GetCurrentTime().c_str());
	} else {
		gNonBlockingMode = false;
		fprintf(tmp.LogFD, "%sServer with 'Blocking' sockets\n",
				tmp.GetCurrentTime().c_str());
	}

	gEncryptionType = gtk_combo_box_get_active(GTK_COMBO_BOX(tmp.GW(
			"TypeServerEncryption")));
	fprintf(tmp.LogFD, "%sServer with '%s' Encryption Type\n",
			tmp.GetCurrentTime().c_str(),
			(char*) gtk_combo_box_get_active_text(GTK_COMBO_BOX(tmp.GW(
					"TypeServerEncryption"))));

	if (!strcmp(strcpy(gPassword, (char*) (GTK_ENTRY(tmp.GW(
			"TypeServerPassword")))->text), "")) {
		markup = g_markup_printf_escaped("<span foreground=\"red\">%s</span>",
				"Login Password:");
		gtk_label_set_markup(GTK_LABEL(tmp.GW("TypeServerL7")), markup);
		g_free(markup);
		InfoCheck = false;
	} else {
		markup = g_markup_printf_escaped(
				"<span foreground=\"black\">%s</span>", "Login Password:");
		gtk_label_set_markup(GTK_LABEL(tmp.GW("TypeServerL7")), markup);
		g_free(markup);
	}
	fprintf(tmp.LogFD, "%sServer Password: %s\n", tmp.GetCurrentTime().c_str(),
			tmp.md5(gPassword).c_str());

	if (InfoCheck)
		tmp.init(gProgramMode, gConnectionType, gIPversion, gIPAddress, gPort,
				gPassword, gNonBlockingMode, gEncryptionType);
	else
		return;

	fprintf(tmp.LogFD, "%sServer Initialized...\n",
			tmp.GetCurrentTime().c_str());
	char* title = new char();
	strcpy(title, "Server: ");
	strcat(title, tmp.HostName);
	gtk_window_set_title(GTK_WINDOW(tmp.GO("ServerWindow")), title);
	gtk_widget_hide(tmp.GW("TypeServerWindow"));
	gtk_widget_show(tmp.GW("ServerWindow"));
}
//----------------------------------------------------------------------------
extern "C" void ConfigureServerCancel(GtkWidget *widget, gpointer data) {
	if (GTK_WINDOW(tmp.GW("TypeServerWindow"))->modal) {
		gtk_widget_hide(tmp.GW("TypeServerWindow"));
		GTK_WINDOW(tmp.GW("TypeServerWindow"))->modal = false;
	} else {
		gtk_widget_hide(tmp.GW("TypeServerWindow"));
		gtk_widget_show(tmp.GW("ModeWindow"));
	}
}
//Configure Server************************************************************
//----------------------------------------------------------------------------
//Configure Client************************************************************
extern "C" void ConfigureClient(GtkWidget *widget, gpointer data) {
	bool InfoCheck = true;
	char *markup;
	if (GTK_TOGGLE_BUTTON(tmp.GW("TypeClientTCP"))->active) {
		strcpy(gConnectionType, "TCP");
	} else if (GTK_TOGGLE_BUTTON(tmp.GW("TypeClientUDP"))->active) {
		strcpy(gConnectionType, "UDP");
	}
	fprintf(tmp.LogFD, "%sClient Connection Type: '%s'\n",
			tmp.GetCurrentTime().c_str(), gConnectionType);

	if (GTK_TOGGLE_BUTTON(tmp.GW("TypeClientIPv4"))->active) {
		strcpy(gIPversion, "IPv4");
	} else if (GTK_TOGGLE_BUTTON(tmp.GW("TypeClientIPv6"))->active) {
		strcpy(gIPversion, "IPv6");
	} else {
		strcpy(gIPversion, "UNSPEC");
	}
	fprintf(tmp.LogFD, "%sClient IP Version: '%s'\n",
			tmp.GetCurrentTime().c_str(), gIPversion);

	if (!strcmp(strcpy(gIPAddress,
			(char*) (GTK_ENTRY(tmp.GW("TypeClientIP")))->text), "")) {
		markup = g_markup_printf_escaped("<span foreground=\"red\">%s</span>",
				"IP:		");
		gtk_label_set_markup(GTK_LABEL(tmp.GW("TypeClientL4")), markup);
		g_free( markup);
		InfoCheck = false;
	} else {
		markup = g_markup_printf_escaped(
				"<span foreground=\"black\">%s</span>", "IP:		");
		gtk_label_set_markup(GTK_LABEL(tmp.GW("TypeClientL4")), markup);
		g_free( markup);
	}
	fprintf(tmp.LogFD, "%sClient IP Address: '%s'\n",
			tmp.GetCurrentTime().c_str(), gIPAddress);

	if (!strcmp(strcpy(gPort,
			(char*) (GTK_ENTRY(tmp.GW("TypeClientPort")))->text), "")) {
		markup = g_markup_printf_escaped("<span foreground=\"red\">%s</span>",
				"Port:	");
		gtk_label_set_markup(GTK_LABEL(tmp.GW("TypeClientL5")), markup);
		g_free( markup);
		InfoCheck = false;
	} else {
		markup = g_markup_printf_escaped(
				"<span foreground=\"black\">%s</span>", "Port:	");
		gtk_label_set_markup(GTK_LABEL(tmp.GW("TypeClientL5")), markup);
		g_free( markup);
	}
	fprintf(tmp.LogFD, "%sClient IP Port: '%s'\n",
			tmp.GetCurrentTime().c_str(), gPort);

	if (GTK_TOGGLE_BUTTON(tmp.GW("TypeClientNonBlock"))->active) {
		gNonBlockingMode = true;
		fprintf(tmp.LogFD, "%sClient with 'NonBlocking' sockets\n",
				tmp.GetCurrentTime().c_str());
	} else {
		gNonBlockingMode = false;
		fprintf(tmp.LogFD, "%sClient with 'Blocking' sockets\n",
				tmp.GetCurrentTime().c_str());
	}

	gEncryptionType = gtk_combo_box_get_active(GTK_COMBO_BOX(tmp.GW(
			"TypeClientEncryption")));
	fprintf(tmp.LogFD, "%sClient with '%s' Encryption Type\n",
			tmp.GetCurrentTime().c_str(),
			(char*) gtk_combo_box_get_active_text(GTK_COMBO_BOX(tmp.GW(
					"TypeClientEncryption"))));

	if (!strcmp(strcpy(tmp.UserName, (char*) (GTK_ENTRY(tmp.GW(
			"TypeClientUserName")))->text), "")) {
		markup = g_markup_printf_escaped("<span foreground=\"red\">%s</span>",
				"User Name:	");
		gtk_label_set_markup(GTK_LABEL(tmp.GW("TypeClientL8")), markup);
		g_free( markup);
		InfoCheck = false;
	} else {
		markup = g_markup_printf_escaped(
				"<span foreground=\"black\">%s</span>", "User Name:	");
		gtk_label_set_markup(GTK_LABEL(tmp.GW("TypeClientL8")), markup);
		g_free( markup);
	}
	fprintf(tmp.LogFD, "%sClient UserName: %s\n", tmp.GetCurrentTime().c_str(),
			tmp.UserName);

	if (!strcmp(strcpy(gPassword, (char*) (GTK_ENTRY(tmp.GW(
			"TypeClientPassword")))->text), "")) {
		markup = g_markup_printf_escaped("<span foreground=\"red\">%s</span>",
				"Password:	");
		gtk_label_set_markup(GTK_LABEL(tmp.GW("TypeClientL9")), markup);
		g_free( markup);
		InfoCheck = false;
	} else {
		markup = g_markup_printf_escaped(
				"<span foreground=\"black\">%s</span>", "Password:	");
		gtk_label_set_markup(GTK_LABEL(tmp.GW("TypeClientL9")), markup);
		g_free( markup);
	}
	fprintf(tmp.LogFD, "%sClient Password: %s\n", tmp.GetCurrentTime().c_str(),
			tmp.md5(gPassword).c_str());

	if (InfoCheck)
		tmp.init(gProgramMode, gConnectionType, gIPversion, gIPAddress, gPort,
				gPassword, gNonBlockingMode, gEncryptionType);
	else
		return;

	fprintf(tmp.LogFD, "%sClient Initialized...\n",
			tmp.GetCurrentTime().c_str());
	char* title = new char();
	strcpy(title, "Client: ");
	strcat(title, tmp.UserName);
	gtk_window_set_title(GTK_WINDOW(tmp.GO("ClientWindow")), title);
	gtk_widget_hide(tmp.GW("TypeClientWindow"));
	gtk_widget_show(tmp.GW("ClientWindow"));
}
extern "C" void ConfigureClientCancel(GtkWidget *widget, gpointer data) {
	if (GTK_WINDOW(tmp.GW("TypeClientWindow"))->modal) {
		gtk_widget_hide(tmp.GW("TypeClientWindow"));
		GTK_WINDOW(tmp.GW("TypeClientWindow"))->modal = false;
	} else {
		gtk_widget_hide(tmp.GW("TypeClientWindow"));
		gtk_widget_show(tmp.GW("ModeWindow"));
	}
}
//----------------------------------------------------------------------------
//Server**********************************************************************
extern "C" void RunServer(GtkWidget *widget, gpointer data) {
	pthread_t th;
	int errcode; /* holds pthread error code */
	if (errcode = pthread_create(&th, NULL, RunServerThread, &tmp)) {
		errexit(errcode,"pthread_create");
	}
	fprintf(tmp.LogFD, "%sServer Started...\n", tmp.GetCurrentTime().c_str());
}
void* RunServerThread(void* arg) {
	((Communications*) arg)->Run();
	pthread_exit(arg);
}
extern "C" void StopServer(GtkWidget *widget, gpointer data) {
	if (tmp.AppStat)
		tmp.FlushAll();
}

extern "C" void DisconnectClients(GtkWidget *widget, gpointer data) {
	tmp.DisconnectClients();
	fprintf(tmp.LogFD, "%sServer Disconnected Clients...\n",
			tmp.GetCurrentTime().c_str());
}

extern "C" void SaveLog(GtkWidget *widget, gpointer data) {
	cout << "Log is autosaving...\n";
}

extern "C" void ServerPreferences(GtkWidget *widget, gpointer data) {
	GTK_WINDOW(tmp.GW("TypeServerWindow"))->modal = true;
	gtk_window_set_transient_for(GTK_WINDOW(tmp.GW("TypeServerWindow")),
			GTK_WINDOW(tmp.GW("ServerWindow")));
	gtk_widget_show(tmp.GW("TypeServerWindow"));
}
//----------------------------------------------------------------------------
//Client**********************************************************************
extern "C" void RunClient(GtkWidget *widget, gpointer data) {
	pthread_t th;
	int errcode; /* holds pthread error code */
	if (errcode = pthread_create(&th, NULL, RunClientThread, &tmp)) {
		errexit(errcode,"pthread_create");
	}
	fprintf(tmp.LogFD, "%sClient Started...\n", tmp.GetCurrentTime().c_str());
}
void* RunClientThread(void* arg) {
	((Communications*) arg)->Run();
	pthread_exit(arg);
}
extern "C" void SendMessage(GtkWidget *widget, gpointer data) {
	pthread_t th;
	int errcode; /* holds pthread error code */
	if (errcode = pthread_create(&th, NULL, SendMessage, &tmp)) {
		errexit(errcode,"pthread_create");
	}
}
void* SendMessage(void* arg) {
	((Communications*) arg)->ClientSendMode();
	gtk_widget_grab_focus(tmp.GW("ClientSendText"));
	gdk_window_process_all_updates();
	pthread_exit(arg);
}
extern "C" void ClientPreferences(GtkWidget *widget, gpointer data) {
	GTK_WINDOW(tmp.GW("TypeClientWindow"))->modal = true;
	gtk_window_set_transient_for(GTK_WINDOW(tmp.GW("TypeClientWindow")),
			GTK_WINDOW(tmp.GW("ClientWindow")));
	gtk_widget_show(tmp.GW("TypeClientWindow"));
}
extern "C" void StopClient(GtkWidget *widget, gpointer data) {
	if (tmp.AppStat)
		tmp.FlushAll();
}
extern "C" void on_ClientSendBuffer_changed(GtkWidget *widget, gpointer data) {
	GtkTextIter start, end;
	GtkTextBuffer *buffer = GTK_TEXT_BUFFER(tmp.GO("ClientSendBuffer"));
	gtk_text_buffer_get_bounds(buffer, &start, &end);
	if (!strcmp((char*) gtk_text_buffer_get_text(buffer, &start, &end, false),
			"")) {
		gtk_widget_set_sensitive(tmp.GW("ClientSend"), false);
	} else {
		gtk_widget_set_sensitive(tmp.GW("ClientSend"), true);
	}
}
//----------------------------------------------------------------------------
extern "C" void print_hello(GtkWidget *widget, gpointer data) {
	g_print("Hello World\n");
	gtk_main_quit();
}

int main(int argc, char *argv[]) {
	//g_thread_init(NULL);
	//gdk_threads_init();
	fprintf(tmp.LogFD, "%sNetworking Program Loaded...\n",
			tmp.GetCurrentTime().c_str());
	tmp.GetMachineIPs();
	gtk_init(&argc, &argv);
	tmp.Builder = gtk_builder_new();
	gtk_builder_add_from_file(tmp.Builder, "Networking.glade", NULL);
	gtk_builder_connect_signals(tmp.Builder, NULL);
	gtk_widget_show_all(tmp.GW("ModeWindow"));
	fprintf(tmp.LogFD, "%sNetworking Program Initialized...\n",
			tmp.GetCurrentTime().c_str());
	//tags****************************************************************
	gtk_text_buffer_create_tag(GTK_TEXT_BUFFER(tmp.GO("ServerTextBuffer")),
			"italic", "style", PANGO_STYLE_ITALIC, NULL);
	gtk_text_buffer_create_tag(GTK_TEXT_BUFFER(tmp.GO("ServerTextBuffer")),
			"bold", "weight", PANGO_WEIGHT_BOLD, NULL);
	gtk_text_buffer_create_tag(GTK_TEXT_BUFFER(tmp.GO("ServerTextBuffer")),
			"error", "background", "white", "foreground", "#C72C20", "size",
			12000, "style", PANGO_STYLE_ITALIC, NULL);
	gtk_text_buffer_create_tag(GTK_TEXT_BUFFER(tmp.GO("ServerTextBuffer")),
			"success", "background", "white", "foreground", "#75A672", "size",
			12000, "style", PANGO_STYLE_ITALIC, NULL);
	gtk_text_buffer_create_tag(GTK_TEXT_BUFFER(tmp.GO("ServerTextBuffer")),
			"lost", "background", "white", "foreground", "#C7A020", "size",
			12000, "style", PANGO_STYLE_ITALIC, NULL);
	gtk_text_buffer_create_tag(GTK_TEXT_BUFFER(tmp.GO("ServerTextBuffer")),
			"time", "background", "white", "foreground", "#B1BEFF", "size",
			9000, NULL);
	gtk_text_buffer_create_tag(GTK_TEXT_BUFFER(tmp.GO("ServerTextBuffer")), "",
			NULL);
	//********************************************************************
	//tags****************************************************************
	gtk_text_buffer_create_tag(GTK_TEXT_BUFFER(tmp.GO("ClientTextBuffer")),
			"italic", "style", PANGO_STYLE_ITALIC, NULL);
	gtk_text_buffer_create_tag(GTK_TEXT_BUFFER(tmp.GO("ClientTextBuffer")),
			"bold", "weight", PANGO_WEIGHT_BOLD, NULL);
	gtk_text_buffer_create_tag(GTK_TEXT_BUFFER(tmp.GO("ClientTextBuffer")),
			"error", "background", "white", "foreground", "#C72C20", "size",
			12000, "style", PANGO_STYLE_ITALIC, NULL);
	gtk_text_buffer_create_tag(GTK_TEXT_BUFFER(tmp.GO("ClientTextBuffer")),
			"success", "background", "white", "foreground", "#75A672", "size",
			12000, "style", PANGO_STYLE_ITALIC, NULL);
	gtk_text_buffer_create_tag(GTK_TEXT_BUFFER(tmp.GO("ClientTextBuffer")),
			"lost", "background", "white", "foreground", "#C7A020", "size",
			12000, "style", PANGO_STYLE_ITALIC, NULL);
	gtk_text_buffer_create_tag(GTK_TEXT_BUFFER(tmp.GO("ClientTextBuffer")),
			"time", "background", "white", "foreground", "#B1BEFF", "size",
			9000, NULL);
	gtk_text_buffer_create_tag(GTK_TEXT_BUFFER(tmp.GO("ClientTextBuffer")), "",
			NULL);
	//********************************************************************
	fprintf(tmp.LogFD, "%sText Tags Added...\n", tmp.GetCurrentTime().c_str());

	//gdk_threads_enter();
	fprintf(tmp.LogFD, "%sNetworking Program Started...\n",
			tmp.GetCurrentTime().c_str());
	gtk_main();
	//gdk_threads_leave();

	fprintf(tmp.LogFD, "%sNetworking Program Ended...\n",
			tmp.GetCurrentTime().c_str());
	cout << "!!!Hello World!!!" << endl; // prints !!!Hello World!!!
	return 0;
}
