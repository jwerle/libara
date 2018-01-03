#ifndef CLAMP_H
#define CLAMP_H

/**
 * Clamps value between min and max values.
 */

#define clamp(value, min, max) (min < max           \
  ? (value < min ? min : value > max ? max : value) \
  : (value < max ? max : value > min ? min : value) \
)

#endif
