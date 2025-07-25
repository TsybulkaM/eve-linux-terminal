#include "eveld.h"
#include <libftdi1/ftdi.h>
#include <unistd.h>

bool check_ftdi_device(void)
{
  struct ftdi_context *ftdi;
  struct ftdi_device_list *devlist;
  int ret;

  if ((ftdi = ftdi_new()) == 0)
  {
    fprintf(stderr, "ftdi_new failed\n");
    return false;
  }

  if ((ret = ftdi_usb_find_all(ftdi, &devlist, 0x1b3d, 0x200)) < 0)
  {
    fprintf(stderr, "ftdi_usb_find_all failed: %d (%s)\n", ret, ftdi_get_error_string(ftdi));
    ftdi_free(ftdi);
    return false;
  }

  bool device_found = (ret > 0);

  ftdi_list_free(&devlist);
  ftdi_free(ftdi);

  return device_found;
}
