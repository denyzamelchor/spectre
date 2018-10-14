# Distributed under the MIT License.
# See LICENSE.txt for details.

import numpy as np


# Functions for testing SmoothFlow.cpp
def rest_mass_density(x, t, mean_velocity, wave_vector, pressure,
                      adiabatic_exponent, density_amplitude):
    return 1.0 + (density_amplitude * np.sin(
        np.dot(
            np.asarray(wave_vector),
            np.asarray(x) - np.asarray(mean_velocity) * t)))


def spatial_velocity(x, t, mean_velocity, wave_vector, pressure,
                     adiabatic_exponent, density_amplitude):
    return np.asarray(mean_velocity)


def specific_internal_energy(x, t, mean_velocity, wave_vector, pressure,
                             adiabatic_exponent, density_amplitude):
    return (pressure / ((adiabatic_exponent - 1.0) * rest_mass_density(
        x, t, mean_velocity, wave_vector, pressure, adiabatic_exponent,
        density_amplitude)))


def pressure(x, t, mean_velocity, wave_vector, pressure, adiabatic_exponent,
             density_amplitude):
    return pressure


def magnetic_field(x, t, mean_velocity, wave_vector, pressure,
                   adiabatic_exponent, density_amplitude):
    return np.array([0.0, 0.0, 0.0])


# End functions for testing SmoothFlow.cpp


# Functions for testing AlfvenWave.cpp
def alfven_rest_mass_density(x, t, wavenumber, pressure, rest_mass_density,
                             adiabatic_exponent, background_mag_field,
                             perturbation_size):
    return rest_mass_density


def alfven_spatial_velocity(x, t, wavenumber, pressure, rest_mass_density,
                            adiabatic_exponent, background_mag_field,
                            perturbation_size):
    rho_zero_times_h = \
     (rest_mass_density + pressure *
      (adiabatic_exponent)/(adiabatic_exponent - 1.0))
    alfven_speed = background_mag_field / \
     np.sqrt(rho_zero_times_h + background_mag_field ** 2)
    phase = wavenumber * (x[2] - alfven_speed * t)
    fluid_velocity = -perturbation_size * alfven_speed / background_mag_field
    return np.array([fluid_velocity * np.cos(phase), \
     fluid_velocity * np.sin(phase), 0.0])


def alfven_specific_internal_energy(x, t, wavenumber, pressure,
                                    rest_mass_density, adiabatic_exponent,
                                    background_mag_field, perturbation_size):
    return pressure / (rest_mass_density * (adiabatic_exponent - 1.0))


def alfven_pressure(x, t, wavenumber, pressure, rest_mass_density,
                    adiabatic_exponent, background_mag_field,
                    perturbation_size):
    return pressure


def alfven_specific_enthalpy(x, t, wavenumber, pressure, rest_mass_density,
                             adiabatic_exponent, background_mag_field,
                             perturbation_size):
    return 1.0 + adiabatic_exponent * alfven_specific_internal_energy(
        x, t, wavenumber, pressure, rest_mass_density, adiabatic_exponent,
        background_mag_field, perturbation_size)


def alfven_lorentz_factor(x, t, wavenumber, pressure, rest_mass_density,
                          adiabatic_exponent, background_mag_field,
                          perturbation_size):
    return 1.0 / np.sqrt(1.0 - np.linalg.norm(
        alfven_spatial_velocity(x, t, wavenumber, pressure, rest_mass_density,
                                adiabatic_exponent, background_mag_field,
                                perturbation_size))**2)


def alfven_magnetic_field(x, t, wavenumber, pressure, rest_mass_density,
                          adiabatic_exponent, background_mag_field,
                          perturbation_size):
    rho_zero_times_h = \
     (rest_mass_density + pressure *
      (adiabatic_exponent)/(adiabatic_exponent - 1.0))
    alfven_speed = background_mag_field / \
     np.sqrt(rho_zero_times_h + background_mag_field ** 2)
    phase = wavenumber * (x[2] - alfven_speed * t)
    fluid_velocity = -perturbation_size * alfven_speed / background_mag_field
    return np.array([perturbation_size * np.cos(phase), \
     perturbation_size * np.sin(phase), background_mag_field])


def alfven_divergence_cleaning_field(x, t, wavenumber, pressure,
                                     rest_mass_density, adiabatic_exponent,
                                     background_mag_field, perturbation_size):
    return 0.0


# End functions for testing AlfvenWave.cpp
