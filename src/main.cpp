#include <iostream>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <X11/Xlib.h>
#include <X11/extensions/XShm.h>
#include <X11/Xutil.h>

const int TEST_SIZE = 512;

static int handleXError(Display *display, XErrorEvent *event)
{
  printf("XErrorEvent triggered!\n");
  printf("error_code: %d", event->error_code);
  printf("minor_code: %d", event->minor_code);
  printf("request_code: %d", event->request_code);
  printf("resourceid: %lu", event->resourceid);
  printf("serial: %d", event->error_code);
  printf("type: %d", event->type);
  return 0;
}

int main()
{
  // Open default display
  Display *display = XOpenDisplay(nullptr);
  int screen = DefaultScreen(display);
  Window rootWin = RootWindow(display, screen);
  GC graphicsContext = DefaultGC(display, screen);

  // Create new window and subscribe to events
  long blackPixel = BlackPixel(display, screen);
  long whitePixel = WhitePixel(display, screen);
  Window newWin = XCreateSimpleWindow(display, rootWin, 0, 0, TEST_SIZE, TEST_SIZE, 1, blackPixel, whitePixel);
  XMapWindow(display, newWin);
  XSelectInput(display, newWin, ExposureMask | KeyPressMask);

  // Allocate shared memory for image capturing
  XShmSegmentInfo shminfo;
  Visual *visual = DefaultVisual(display, screen);
  int depth = DefaultDepth(display, screen);
  XImage *image = XShmCreateImage(display, visual, depth, ZPixmap, nullptr, &shminfo, TEST_SIZE, TEST_SIZE);
  shminfo.shmid = shmget(IPC_PRIVATE, image->bytes_per_line * image->height, IPC_CREAT | 0777);
  image->data = (char*)shmat(shminfo.shmid, 0, 0);
  shminfo.shmaddr = image->data;
  XSetErrorHandler(handleXError);
  shminfo.readOnly = False;
  XShmAttach(display, &shminfo);
  XSync(display, false);
  shmctl(shminfo.shmid, IPC_RMID, 0);

  // Main event loop for new window
  XEvent event;
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
    if(image->data != nullptr) // NEVER TRUE.  DATA IS ALWAYS NULL!
    {
      long pixel = 0;
      for (int x = 0; x < image->width; x++)
      {
        for (int y = 0; y < image->height; y++)
        {
          // Invert the color of each pixel
          pixel = XGetPixel(image, x, y);
          XPutPixel(image, x, y, ~pixel);
        }
      }
    }

    // Output the modified image
    if (exposed && killWindow == false)
    {
      XShmPutImage(display, newWin, graphicsContext, image, 0, 0, 0, 0, 512, 512, false);
    }
  }

  // Goodbye
  XShmDetach(display, &shminfo);
  XDestroyImage(image);
  shmdt(shminfo.shmaddr);
  XCloseDisplay(display);
}
