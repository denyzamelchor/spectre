// Distributed under the MIT License.
// See LICENSE.txt for details.

#include "tests/Unit/TestingFramework.hpp"

#include <array>
#include <cstddef>
#include <random>

#include "DataStructures/DataVector.hpp"
#include "DataStructures/Tensor/Tensor.hpp"
#include "Evolution/Systems/GeneralizedHarmonic/Equations.hpp"
#include "Utilities/Gsl.hpp"

// IWYU pragma: no_forward_declare Tensor

namespace {
template <typename Tensor>
Tensor create_tensor_with_random_values(
    const size_t n_pts, const gsl::not_null<std::mt19937*> gen) {
  Tensor tensor(n_pts);
  for (auto& data : tensor) {
    for (size_t s = 0; s < data.size(); ++s) {
      data[s] = std::uniform_real_distribution<>(-10, 10)(*gen);
    }
  }
  return tensor;
}
}  // namespace

SPECTRE_TEST_CASE("Unit.Evolution.Systems.GeneralizedHarmonic.DuDt",
                  "[Unit][GeneralizedHarmonic]") {
  const size_t dim = 3;
  const size_t n_pts = 2;

  // This test compares the output of the GeneralizedHarmonic RHS to the output
  // computed using SpEC for the specific input tensors generated by this seed.
  std::mt19937 gen(1.);

  tnsr::aa<DataVector, dim, Frame::Inertial> dt_psi(n_pts);
  tnsr::aa<DataVector, dim, Frame::Inertial> dt_pi(n_pts);
  tnsr::iaa<DataVector, dim, Frame::Inertial> dt_phi(n_pts);

  const auto psi = create_tensor_with_random_values<
      tnsr::aa<DataVector, dim, Frame::Inertial>>(n_pts, make_not_null(&gen));
  const auto pi = create_tensor_with_random_values<
      tnsr::aa<DataVector, dim, Frame::Inertial>>(n_pts, make_not_null(&gen));
  const auto phi = create_tensor_with_random_values<
      tnsr::iaa<DataVector, dim, Frame::Inertial>>(n_pts, make_not_null(&gen));
  const auto d_psi = create_tensor_with_random_values<
      tnsr::iaa<DataVector, dim, Frame::Inertial>>(n_pts, make_not_null(&gen));
  const auto d_pi = create_tensor_with_random_values<
      tnsr::iaa<DataVector, dim, Frame::Inertial>>(n_pts, make_not_null(&gen));
  const auto d_phi = create_tensor_with_random_values<
      tnsr::ijaa<DataVector, dim, Frame::Inertial>>(n_pts, make_not_null(&gen));
  const auto gauge_function = create_tensor_with_random_values<
      tnsr::a<DataVector, dim, Frame::Inertial>>(n_pts, make_not_null(&gen));
  const auto spacetime_deriv_gauge_function = create_tensor_with_random_values<
      tnsr::ab<DataVector, dim, Frame::Inertial>>(n_pts, make_not_null(&gen));
  const auto gamma0 = create_tensor_with_random_values<Scalar<DataVector>>(
      n_pts, make_not_null(&gen));
  const auto gamma1 = create_tensor_with_random_values<Scalar<DataVector>>(
      n_pts, make_not_null(&gen));
  const auto gamma2 = create_tensor_with_random_values<Scalar<DataVector>>(
      n_pts, make_not_null(&gen));
  const auto lapse = create_tensor_with_random_values<Scalar<DataVector>>(
      n_pts, make_not_null(&gen));
  const auto shift = create_tensor_with_random_values<
      tnsr::I<DataVector, dim, Frame::Inertial>>(n_pts, make_not_null(&gen));
  const auto upper_spatial_metric = create_tensor_with_random_values<
      tnsr::II<DataVector, dim, Frame::Inertial>>(n_pts, make_not_null(&gen));
  const auto upper_psi = create_tensor_with_random_values<
      tnsr::AA<DataVector, dim, Frame::Inertial>>(n_pts, make_not_null(&gen));
  const auto christoffel_first_kind = create_tensor_with_random_values<
      tnsr::abb<DataVector, dim, Frame::Inertial>>(n_pts, make_not_null(&gen));
  const auto christoffel_second_kind = create_tensor_with_random_values<
      tnsr::Abb<DataVector, dim, Frame::Inertial>>(n_pts, make_not_null(&gen));
  const auto trace_christoffel_first_kind = create_tensor_with_random_values<
      tnsr::a<DataVector, dim, Frame::Inertial>>(n_pts, make_not_null(&gen));
  const auto normal_one_form = create_tensor_with_random_values<
      tnsr::a<DataVector, dim, Frame::Inertial>>(n_pts, make_not_null(&gen));
  const auto normal_vector = create_tensor_with_random_values<
      tnsr::A<DataVector, dim, Frame::Inertial>>(n_pts, make_not_null(&gen));

  GeneralizedHarmonic::ComputeDuDt<dim>::apply(
      make_not_null(&dt_psi), make_not_null(&dt_pi), make_not_null(&dt_phi),
      psi, pi, phi, d_psi, d_pi, d_phi, gamma0, gamma1, gamma2, gauge_function,
      spacetime_deriv_gauge_function, lapse, shift, upper_spatial_metric,
      upper_psi, trace_christoffel_first_kind, christoffel_first_kind,
      christoffel_second_kind, normal_vector, normal_one_form);

  CHECK(dt_psi.get(0, 0)[0] == approx(-488.874963261792004));
  CHECK(dt_psi.get(0, 0)[1] == approx(-81.510619345029468));
  CHECK(dt_psi.get(0, 1)[0] == approx(-250.047822240104125));
  CHECK(dt_psi.get(0, 1)[1] == approx(296.302836131049901));
  CHECK(dt_psi.get(0, 2)[0] == approx(-500.997780531678018));
  CHECK(dt_psi.get(0, 2)[1] == approx(466.785608622853886));
  CHECK(dt_psi.get(0, 3)[0] == approx(467.671854125188190));
  CHECK(dt_psi.get(0, 3)[1] == approx(495.703587199368314));
  CHECK(dt_psi.get(1, 1)[0] == approx(-1455.466926116829427));
  CHECK(dt_psi.get(1, 1)[1] == approx(537.232777225229825));
  CHECK(dt_psi.get(1, 2)[0] == approx(605.305755751280685));
  CHECK(dt_psi.get(1, 2)[1] == approx(-147.555027862059319));
  CHECK(dt_psi.get(1, 3)[0] == approx(272.588366899036828));
  CHECK(dt_psi.get(1, 3)[1] == approx(103.726287040730483));
  CHECK(dt_psi.get(2, 2)[0] == approx(-698.404763643118372));
  CHECK(dt_psi.get(2, 2)[1] == approx(-54.529887031214074));
  CHECK(dt_psi.get(2, 3)[0] == approx(685.972755990463384));
  CHECK(dt_psi.get(2, 3)[1] == approx(354.613491895308584));
  CHECK(dt_psi.get(3, 3)[0] == approx(-245.215344706651052));
  CHECK(dt_psi.get(3, 3)[1] == approx(-276.589507479549297));
  CHECK(dt_pi.get(0, 0)[0] == approx(-1994.203229187604393));
  CHECK(dt_pi.get(0, 0)[1] == approx(-525201.290706016356125));
  CHECK(dt_pi.get(0, 1)[0] == approx(6311.905865276433360));
  CHECK(dt_pi.get(0, 1)[1] == approx(-240190.350753725302638));
  CHECK(dt_pi.get(0, 2)[0] == approx(-7434.327351519351396));
  CHECK(dt_pi.get(0, 2)[1] == approx(-252008.364267619152088));
  CHECK(dt_pi.get(0, 3)[0] == approx(2838.324638613014940));
  CHECK(dt_pi.get(0, 3)[1] == approx(51236.973434082428867));
  CHECK(dt_pi.get(1, 1)[0] == approx(-354.079220430901557));
  CHECK(dt_pi.get(1, 1)[1] == approx(126094.565118705097120));
  CHECK(dt_pi.get(1, 2)[0] == approx(-5619.191809738355005));
  CHECK(dt_pi.get(1, 2)[1] == approx(-107407.264584976015612));
  CHECK(dt_pi.get(1, 3)[0] == approx(897.475897012644054));
  CHECK(dt_pi.get(1, 3)[1] == approx(89268.754206339028315));
  CHECK(dt_pi.get(2, 2)[0] == approx(-21157.862553074402967));
  CHECK(dt_pi.get(2, 2)[1] == approx(-94493.937608642052510));
  CHECK(dt_pi.get(2, 3)[0] == approx(-6057.203556770715295));
  CHECK(dt_pi.get(2, 3)[1] == approx(-153608.523542909941170));
  CHECK(dt_pi.get(3, 3)[0] == approx(-17537.832989340076892));
  CHECK(dt_pi.get(3, 3)[1] == approx(-60984.946935138112167));
  CHECK(dt_phi.get(0, 0, 0)[0] == approx(334.313398747739541));
  CHECK(dt_phi.get(0, 0, 0)[1] == approx(25514.306297616123629));
  CHECK(dt_phi.get(0, 0, 1)[0] == approx(2077.905394153287034));
  CHECK(dt_phi.get(0, 0, 1)[1] == approx(-23798.504669162444770));
  CHECK(dt_phi.get(0, 0, 2)[0] == approx(-1284.875746578147073));
  CHECK(dt_phi.get(0, 0, 2)[1] == approx(-19831.646993788559485));
  CHECK(dt_phi.get(0, 0, 3)[0] == approx(233.700445801117098));
  CHECK(dt_phi.get(0, 0, 3)[1] == approx(-34774.544349022733513));
  CHECK(dt_phi.get(0, 1, 1)[0] == approx(885.722636195821224));
  CHECK(dt_phi.get(0, 1, 1)[1] == approx(-33932.040869534444937));
  CHECK(dt_phi.get(0, 1, 2)[0] == approx(-650.957577518966218));
  CHECK(dt_phi.get(0, 1, 2)[1] == approx(2638.696086360518620));
  CHECK(dt_phi.get(0, 1, 3)[0] == approx(31.485299569095631));
  CHECK(dt_phi.get(0, 1, 3)[1] == approx(-3754.688091563506987));
  // This value is only accurate to within 1e-13 when built on release with GCC
  // 6.3
  Approx coarser_approx = Approx::custom().epsilon(1e-13);
  CHECK(dt_phi.get(0, 2, 2)[0] == coarser_approx(10.787542001317307));
  CHECK(dt_phi.get(0, 2, 2)[1] == approx(6388.265784499862093));
  CHECK(dt_phi.get(0, 2, 3)[0] == approx(-264.672203939199221));
  CHECK(dt_phi.get(0, 2, 3)[1] == approx(-14106.605264503405124));
  CHECK(dt_phi.get(0, 3, 3)[0] == approx(-186.705594181490596));
  CHECK(dt_phi.get(0, 3, 3)[1] == approx(37619.063591638048820));
  CHECK(dt_phi.get(1, 0, 0)[0] == approx(-562.600849624935677));
  CHECK(dt_phi.get(1, 0, 0)[1] == approx(26636.175386835606332));
  CHECK(dt_phi.get(1, 0, 1)[0] == approx(-7348.564617671276210));
  CHECK(dt_phi.get(1, 0, 1)[1] == approx(-10239.582656574073553));
  CHECK(dt_phi.get(1, 0, 2)[0] == approx(3735.515737741963676));
  CHECK(dt_phi.get(1, 0, 2)[1] == approx(-9541.966102859829334));
  CHECK(dt_phi.get(1, 0, 3)[0] == approx(-189.717363255539595));
  CHECK(dt_phi.get(1, 0, 3)[1] == approx(-38768.276299298151571));
  CHECK(dt_phi.get(1, 1, 1)[0] == approx(-3992.834118054164264));
  CHECK(dt_phi.get(1, 1, 1)[1] == approx(-27052.090043482003239));
  CHECK(dt_phi.get(1, 1, 2)[0] == approx(3861.313476781318514));
  CHECK(dt_phi.get(1, 1, 2)[1] == approx(9909.233790204545585));
  CHECK(dt_phi.get(1, 1, 3)[0] == approx(781.120763452604706));
  CHECK(dt_phi.get(1, 1, 3)[1] == approx(32796.160467693429382));
  CHECK(dt_phi.get(1, 2, 2)[0] == approx(-1182.382985415981693));
  CHECK(dt_phi.get(1, 2, 2)[1] == approx(-19585.499004859902925));
  CHECK(dt_phi.get(1, 2, 3)[0] == approx(2533.123508025803858));
  CHECK(dt_phi.get(1, 2, 3)[1] == approx(39702.781747730077768));
  CHECK(dt_phi.get(1, 3, 3)[0] == approx(1356.930486013502104));
  CHECK(dt_phi.get(1, 3, 3)[1] == approx(18614.741669036127860));
  CHECK(dt_phi.get(2, 0, 0)[0] == approx(-267.727784019130809));
  CHECK(dt_phi.get(2, 0, 0)[1] == approx(-34312.754364113869087));
  CHECK(dt_phi.get(2, 0, 1)[0] == approx(-3628.105306092249975));
  CHECK(dt_phi.get(2, 0, 1)[1] == approx(26021.001208727309859));
  CHECK(dt_phi.get(2, 0, 2)[0] == approx(1787.972861399919111));
  CHECK(dt_phi.get(2, 0, 2)[1] == approx(20245.691906571166328));
  CHECK(dt_phi.get(2, 0, 3)[0] == approx(-345.850954809957727));
  CHECK(dt_phi.get(2, 0, 3)[1] == approx(45631.496531139637227));
  CHECK(dt_phi.get(2, 1, 1)[0] == approx(-1936.213815715472037));
  CHECK(dt_phi.get(2, 1, 1)[1] == approx(39876.740245914304978));
  CHECK(dt_phi.get(2, 1, 2)[0] == approx(2047.144489678545369));
  CHECK(dt_phi.get(2, 1, 2)[1] == approx(-6162.040420810454634));
  CHECK(dt_phi.get(2, 1, 3)[0] == approx(490.866985829649764));
  CHECK(dt_phi.get(2, 1, 3)[1] == approx(-5369.075417735416522));
  CHECK(dt_phi.get(2, 2, 2)[0] == approx(-444.587399207923738));
  CHECK(dt_phi.get(2, 2, 2)[1] == approx(1205.339390995000485));
  CHECK(dt_phi.get(2, 2, 3)[0] == approx(1208.970864104782549));
  CHECK(dt_phi.get(2, 2, 3)[1] == approx(1422.401000428901625));
  CHECK(dt_phi.get(2, 3, 3)[0] == approx(1116.070338196526109));
  CHECK(dt_phi.get(2, 3, 3)[1] == approx(-42638.998279054998420));
}
