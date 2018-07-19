#include <iostream>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <X11/Xlib.h>
#include <X11/extensions/XShm.h>
#include <X11/Xutil.h>

const int TEST_SIZE = 512;

int main()
{
  // Open default display
  Display *display = XOpenDisplay(nullptr);
  int screen = DefaultScreen(display);
  Window rootWin = RootWindow(display, screen);
  GC graphicsContext = DefaultGC(display, screen);

  // Create new window and subscribe to events
  Window newWin = XCreateSimpleWindow(display, rootWin, 0, 0, TEST_SIZE, TEST_SIZE, 1, BlackPixel(display, screen),
                                      WhitePixel(display, screen));
  XMapWindow(display, newWin);
  XSelectInput(display, newWin, ExposureMask | KeyPressMask);
  XEvent event;

  // Allocate shared memory for image capturing
  Visual *visual = DefaultVisual(display, 0);
  XShmSegmentInfo shminfo;
  int depth = visual->bits_per_rgb * 3;
  XImage *image = XShmCreateImage(display, visual, depth, ZPixmap, NULL, &shminfo, TEST_SIZE, TEST_SIZE);
  shminfo.shmid = shmget(IPC_PRIVATE, image->bytes_per_line * image->height, IPC_CREAT | 0666);
  shmat(shminfo.shmid, nullptr, 0);
  shminfo.shmaddr = image->data;
  shminfo.readOnly = False;
  XShmAttach(display, &shminfo);

  // Main event loop for window
  bool exposed = false;
  bool killWindow = false;
  while (!killWindow)
  {
    // Handle pending events
    if (XPending(display) > 0)
    {
      XNextEvent(display, &event);
      if (event.type == Expose)
      {
        exposed = true;
      } else if (event.type == NoExpose)
      {
        exposed = false;
      } else if (event.type == KeyPress)
      {
        killWindow = true;
      }
    }

    // Capture the original image
    XShmGetImage(display, rootWin, image, 0, 0, AllPlanes);

    // Modify the image
    if(image->data != nullptr)
    {
      long pixel = 0;
      for (int x = 0; x < image->width; x++)
      {
        for (int y = 0; y < image->height; y++)
        {
          // TODO: Figure out why image->data is always null
          pixel = XGetPixel(image, x, y);

          // TODO: Verify that this mechanism for inverting the colors actually works.
          XPutPixel(image, x, y, ~pixel);
        }
      }
    }

    // Output the modified image
    if (exposed && killWindow == false)
    {
      XShmPutImage(display, newWin, graphicsContext, image, 0, 0, 0, 0, 512, 512, true);
    }
  }

  // Goodbye
  XFree(image);
  XCloseDisplay(display);
}
