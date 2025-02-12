#ifndef CDF97_H
#define CDF97_H

#include <cmath>

#include "sperr_helper.h"

namespace sperr {

class CDF97 {
 public:
  //
  // Input
  //
  // Note that copy_data() and take_data() effectively resets internal states
  // of this class.
  template <typename T>
  auto copy_data(const T* buf, size_t len, dims_type dims) -> RTNType;
  auto take_data(std::vector<double>&& buf, dims_type dims) -> RTNType;

  //
  // Output
  //
  auto view_data() const -> const std::vector<double>&;
  auto release_data() -> std::vector<double>&&;
  auto get_dims() const -> std::array<size_t, 3>;  // In 2D case, the 3rd value equals 1.

  // Action items
  void dwt1d();
  void idwt1d();
  void dwt2d();   // 1) calculates the number of levels of dwt,
                  // 2) perform the actual dwt.
  void idwt2d();  // 1) calculates the number of levels of dwt,
                  // 2) perform the actual idwt
  void dwt3d_wavelet_packet();
  void idwt3d_wavelet_packet();
  void dwt3d_dyadic();
  void idwt3d_dyadic();

 private:
  using itd_type = vecd_type::iterator;
  using citd_type = vecd_type::iterator;

  //
  // Private methods helping DWT.
  //

  // Multiple levels of 1D DWT/IDWT on a given array of length array_len.
  void m_dwt1d(itd_type array, size_t array_len, size_t num_of_xforms);
  void m_idwt1d(itd_type array, size_t array_len, size_t num_of_xforms);

  // Multiple levels of 2D DWT/IDWT on a given plane by repeatedly invoking
  // m_dwt2d_one_level(). The plane has a dimension (len_xy[0], len_xy[1]).
  void m_dwt2d(itd_type plane, std::array<size_t, 2> len_xy, size_t num_of_xforms);
  void m_idwt2d(itd_type plane, std::array<size_t, 2> len_xy, size_t num_of_xforms);

  // Perform one level of interleaved 3D dwt/idwt on a given volume (m_dims),
  // specifically on its top left (len_xyz) subset.
  void m_dwt3d_one_level(itd_type vol, std::array<size_t, 3> len_xyz);
  void m_idwt3d_one_level(itd_type vol, std::array<size_t, 3> len_xyz);

  // Perform one level of 2D dwt/idwt on a given plane (m_dims),
  // specifically on its top left (len_xy) subset.
  void m_dwt2d_one_level(itd_type plane, std::array<size_t, 2> len_xy);
  void m_idwt2d_one_level(itd_type plane, std::array<size_t, 2> len_xy);

  // Perform one level of 1D dwt/idwt on a given array (array_len).
  // A buffer space (tmp_buf) should be passed in for
  // this method to work on with length at least 2*array_len.
  void m_dwt1d_one_level(itd_type array, size_t array_len);
  void m_idwt1d_one_level(itd_type array, size_t array_len);

  // Separate even and odd indexed elements to be at the front and back of the dest array.
  // Note 1: sufficient memory space should be allocated by the caller.
  // Note 2: two versions for even and odd length input.
  void m_gather_even(citd_type begin, citd_type end, itd_type dest) const;
  void m_gather_odd(citd_type begin, citd_type end, itd_type dest) const;

  // Interleave low and high pass elements to be at even and odd positions of the dest array.
  // Note 1: sufficient memory space should be allocated by the caller.
  // Note 2: two versions for even and odd length input.
  void m_scatter_even(citd_type begin, citd_type end, itd_type dest) const;
  void m_scatter_odd(citd_type begin, citd_type end, itd_type dest) const;

  //
  // Methods from QccPack, so keep their original names, interface, and the use
  // of raw pointers.
  //
  void QccWAVCDF97AnalysisSymmetricEvenEven(double* signal, size_t signal_length);
  void QccWAVCDF97AnalysisSymmetricOddEven(double* signal, size_t signal_length);
  void QccWAVCDF97SynthesisSymmetricEvenEven(double* signal, size_t signal_length);
  void QccWAVCDF97SynthesisSymmetricOddEven(double* signal, size_t signal_length);

  //
  // Private data members
  //
  vecd_type m_data_buf;          // Holds the entire input data.
  dims_type m_dims = {0, 0, 0};  // Dimension of the data volume

  // Temporary buffers that are big enough for any (1D column * 2) or any 2D
  // slice. Note: `m_qcc_buf` should be used by m_***_one_level() functions and
  // should not be used by higher-level functions. `m_slice_buf` is only used by
  // wavelet-packet transforms.
  vecd_type m_qcc_buf;
  vecd_type m_slice_buf;

  /*
   * Note on the coefficients and constants:
   * The ones from QccPack are slightly different from what's described in the
   * lifting scheme paper: Pg19 of "FACTORING WAVELET TRANSFORMS INTO LIFTING
   * STEPS," DAUBECHIES and SWELDEN.
   * (https://9p.io/who/wim/papers/factor/factor.pdf)
   * JasPer, OpenJPEG, and FFMpeg use coefficients closer to the paper.
   * The filter bank coefficients (h[] array) are from "Biorthogonal Bases of
   * Compactly Supported Wavelets," by Cohen et al., Page 551.
   * (https://services.math.duke.edu/~ingrid/publications/CPAM_1992_p485.pdf)
   */

  // Paper coefficients
  const double h[5]{.602949018236, .266864118443, -.078223266529, -.016864118443, .026748757411};
  const double r0 = h[0] - 2.0 * h[4] * h[1] / h[3];
  const double r1 = h[2] - h[4] - h[4] * h[1] / h[3];
  const double s0 = h[1] - h[3] - h[3] * r0 / r1;
  const double t0 = h[0] - 2.0 * (h[2] - h[4]);
  const double ALPHA = h[4] / h[3];
  const double BETA = h[3] / r1;
  const double GAMMA = r1 / s0;
  const double DELTA = s0 / t0;
  const double EPSILON = std::sqrt(2.0) * t0;
  const double INV_EPSILON = 1.0 / (std::sqrt(2.0) * t0);

  // QccPack coefficients
  /*
   * const double ALPHA   = -1.58615986717275;
   * const double BETA    = -0.05297864003258;
   * const double GAMMA   =  0.88293362717904;
   * const double DELTA   =  0.44350482244527;
   * const double EPSILON =  1.14960430535816;
   */
};

};  // namespace sperr

#endif
