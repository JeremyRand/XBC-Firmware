#ifndef __ASM_MACROS_H
#define __ASM_MACROS_H



#define FUNCTION(label) .global label ; .type label, function ; .arm ; label:
#define THUMB_FUNCTION(label) .global label ; .type label, function ; .thumb_func ; label:

#define STATIC_FUNCTION(label)  .type label, function ; .arm ; label:
#define STATIC_THUMB_FUNCTION(label) .type label, function ; .thumb_func ; label:

#define OBJECT(label)  .global label ; .type label, object ; label:
#define STATIC_OBJECT(label)  .type label, object ; label:
#define WEAK_OBJECT(label)  .weak label ; .type label, object ; label:

#define END(label)  .L##label##_end: ; .size label, .L##label##_end - label

#if defined(__thumb__)
  #if defined(__THUMB_INTERWORK__)
    #define THUMB_INTERWORK
  #else
    #define THUMB_NO_INTERWORK
  #endif
#else
  #if defined(__THUMB_INTERWORK__)
    #define ARM_INTERWORK
  #else
    #define ARM_NO_INTERWORK
  #endif
#endif



#endif // __ASM_MACROS_H

