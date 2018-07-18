#include <iostream>
#include <X11/Xlib.h>

int main()
{
  Display *display;
  display = XOpenDisplay(NULL);
  if (!display)
  {
    fprintf(stderr, "unable to connect to display");
    return 7;
  }
  Window w;
  int x, y, i;
  unsigned m;
  Window root = XDefaultRootWindow(display);
  if (!root)
  {
    fprintf(stderr, "unable to open rootwindow");
    return 8;
  }
  if (!XQueryPointer(display, root, &root, &w, &x, &y, &i, &i, &m))
  {
    printf("unable to query pointer\n");
    return 9;
  }
  XImage *image;
  XWindowAttributes attr;
  XGetWindowAttributes(display, root, &attr);
  image = XGetImage(display, root, 0, 0, attr.width, attr.height, AllPlanes, XYPixmap);
  XCloseDisplay(display);
  if (!image)
  {
    printf("unable to get image\n");
    return 10;
  }
}
