#ifndef UTIL_MATHUTIL_
#define UTIL_MATHUTIL_

namespace util {

template <typename T>
T map_range(T value, T start, T end, T mapped_start, T mapped_end) {
  if (start == end) return mapped_start;

  double proportion = static_cast<double>(value - start) / (end - start);

  return static_cast<T>(mapped_start +
                        proportion * (mapped_end - mapped_start));
}

}  // namespace util

#endif /* UTIL_MATHUTIL_ */
