/* kernel space to/from user space copy functions */

#include <tk/tkernel.h>

#include <sys/types.h>
#include "libkern.h"
#include <t2ex/errno.h>

#define TKN_COPY_CHECK

/* copyin: copy kernel space (no logical space) from user space */
int
copyin(const void *uaddr, void *kaddr, size_t len)
{
#ifdef TKN_COPY_CHECK
  ER ercd;
  // check address validation
  ercd = ChkSpaceR((VP)uaddr, len);
  if (ercd != E_OK)
    return EFAULT;
#if 0
  ercd = LockSpace((VP)uaddr, len);
  if (ercd != E_OK)
    return EFAULT;
#endif
#endif

  memcpy(kaddr, uaddr, len);

#ifdef TKN_COPY_CHECK
#if 0
  UnlockSpace((VP)uaddr, len);
#endif
#endif

  return 0;
}

/* copyout: copy kernel space (no logical space) to user space */
int
copyout(const void *kaddr, void *uaddr, size_t len)
{
#ifdef TKN_COPY_CHECK
  ER ercd;
  //  check address validation
  ercd = ChkSpaceRW(uaddr, len);
  if (ercd != E_OK)
    return EFAULT;
#if 0
  ercd = LockSpace(uaddr, len);
  if (ercd != E_OK)
    return EFAULT;
#endif
#endif

  memcpy(uaddr, kaddr, len);

#ifdef TKN_COPY_CHECK
#if 0
  UnlockSpace(uaddr, len);
#endif
#endif

  return 0;
}
