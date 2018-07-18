#include <iostream>
#include <X11/Xlib.h>
#include <cstring>
#include <string>

void existenceCheck(void* checkObject, const char* errorMessage, Display* display)
{
  if(!checkObject) {
    fprintf(stderr, "%s", errorMessage);
    if(display)
    {
      XCloseDisplay(display);
    }
    exit(1);
  }
}

int main()
{
  // Open default display
  Display* display = XOpenDisplay(NULL);
  existenceCheck(display, "Unable to open display!", display);

  // Record screen and root window attributes
  int screen = DefaultScreen(display);
  Window rootWin = RootWindow(display, screen);
  XWindowAttributes rootWinAttr;
  XGetWindowAttributes(display, rootWin, &rootWinAttr);
  GC graphicsContext = DefaultGC(display, screen);

  // Capture image from display
  XImage* image = XGetImage(display, rootWin, 0, 0, rootWinAttr.width, rootWinAttr.height, AllPlanes, ZPixmap);
  existenceCheck(image, "Unable to capture image!", display);

  // Create new window and subscribe to events
  Window newWin = XCreateSimpleWindow(display, rootWin, 10, 10, 100, 100, 1, BlackPixel(display, screen), WhitePixel(display, screen));
  XMapWindow(display, newWin);
  XSelectInput(display, newWin, ExposureMask | KeyPressMask);
  XEvent event;

  // Main event loop for window
  const char *message = "Hello, World!";
  while (True)
  {
    XNextEvent(display, &event);
    if (event.type == Expose)
    {
      XFillRectangle(display, newWin, graphicsContext, 20, 20, 10, 10);
      XDrawString(display, newWin, graphicsContext, 10, 50, message, strlen(message));
    }
    else if (event.type == KeyPress)
    {
      break;
    }
  }

  // Goodbye
  XCloseDisplay(display);
}
