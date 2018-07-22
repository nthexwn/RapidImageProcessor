    #include <iostream>
    #include <sys/ipc.h>
    #include <sys/shm.h>
    #include <X11/Xlib.h>
    #include <X11/extensions/XShm.h>
    #include <X11/Xutil.h>

    const int TEST_SIZE = 16;

    static int handleXError(Display *display, XErrorEvent *event)
    {
      printf("XErrorEvent triggered!\n");
      printf("error_code: %d\n", event->error_code);
      printf("minor_code: %d\n", event->minor_code);
      printf("request_code: %d\n", event->request_code);
      printf("resourceid: %lu\n", event->resourceid);
      printf("serial: %d\n", event->error_code);
      printf("type: %d\n", event->type);
      return 0;
    }

    int main()
    {
      // Open default display
      XSetErrorHandler(handleXError);
      Display *display = XOpenDisplay(nullptr);
      int screen = DefaultScreen(display);
      Window rootWin = RootWindow(display, screen);
      GC graphicsContext = DefaultGC(display, screen);

      // Find window to capture from
      Window qRoot;
      Window qChild;
      Window sourceWin;
      int qRootX;
      int qRootY;
      int qChildX;
      int qChildY;
      unsigned int qMask;
      bool windowSelected = false;
      bool mouseDown = false;
      while(!windowSelected)
      {
        if(XQueryPointer(display, rootWin, &qRoot, &qChild, &qRootX, &qRootY, &qChildX, &qChildY, &qMask))
        {
          if(qMask & 256)
          {
            mouseDown = true;
          }
          else if(mouseDown)
          {
            sourceWin = qChild;
            windowSelected = true;
          }
        }
      }
      XWindowAttributes sourceAttributes;
      XGetWindowAttributes(display, sourceWin, &sourceAttributes);
      int sourceWidth = sourceAttributes.width;
      int sourceHeight = sourceAttributes.height;

      // Create new window and subscribe to events
      long blackPixel = BlackPixel(display, screen);
      long whitePixel = WhitePixel(display, screen);
      Window targetWin = XCreateSimpleWindow(display, rootWin, 0, 0, sourceWidth, sourceHeight, 1, blackPixel, whitePixel);
      XMapWindow(display, targetWin);
      XSelectInput(display, targetWin, ExposureMask | KeyPressMask);

      // Allocate shared memory for image capturing
      XShmSegmentInfo shminfo;
      Visual *visual = DefaultVisual(display, screen);
      int depth = DefaultDepth(display, screen);
      XImage *image = XShmCreateImage(display, visual, depth, ZPixmap, nullptr, &shminfo, sourceWidth, sourceHeight);
      shminfo.shmid = shmget(IPC_PRIVATE, image->bytes_per_line * image->height, IPC_CREAT | 0777);
      image->data = (char*)shmat(shminfo.shmid, 0, 0);
      shminfo.shmaddr = image->data;
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
        XShmGetImage(display, sourceWin, image, 0, 0, AllPlanes);

        // Modify the image
        if(image->data != nullptr)
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
          XShmPutImage(display, targetWin, graphicsContext, image, 0, 0, 0, 0, sourceWidth, sourceHeight, false);
        }
      }

      // Goodbye
      XShmDetach(display, &shminfo);
      XDestroyImage(image);
      shmdt(shminfo.shmaddr);
      XCloseDisplay(display);
    }
