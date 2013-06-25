/*
 * Graphics.h
 *
 *  Created on: Aug 13, 2011
 *      Author: ben
 */

#ifndef GRAPHICS_H_
#define GRAPHICS_H_

#include <iostream>
using namespace std;

#include <stdio.h>	//Keyboard & Monitor
#include <stdlib.h>	//exit(1) & atoi()
#include <string.h>
#include <sstream>

#include <time.h>

#include <errno.h>

#include <gtk-2.0/gtk/gtk.h>
#include <pthread.h>

#define SIter(buffer,Iter) gtk_text_buffer_get_start_iter(buffer, &Iter);
#define EIter(buffer,Iter) gtk_text_buffer_get_end_iter(buffer, &Iter);

#define easyLog(tag)\
	cout<<Message;\
	BufferInsert(tag);\
	fprintf(LogFD,"%s%s", GetCurrentTime().c_str(),Message);

class Graphics {
private:
	void LogInit();

protected:
	char ProgramMode[10];
	char *Message;
	GtkTextBuffer *ServerTextBuffer, *ClientTextBuffer, *ClientSendBuffer;

public:
	FILE *LogFD;
	GtkBuilder* Builder;

	GObject* GO(const char* objectname);
	GtkWidget* GW(const char* widgetname);

	void BufferInsert(const char *tag);
	char* BufferGet();

	void quick_message(const char *message);

	void ClientCheckUpdate(const char buf[256]);
	void ClientCheck(const int num, const char name[20]);
	char* getClientCheck();

	string GetCurrentTime();

	Graphics();
	virtual ~Graphics();
};

#endif /* GRAPHICS_H_ */

