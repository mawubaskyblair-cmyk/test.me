"""
1_atmosphere_simulation.py
--------------------------
Calculates atmospheric properties (Temperature, Pressure, Density, Speed of Sound)
from Sea Level up to the Karman Line (100 km) using the U.S. Standard Atmosphere Model.
"""

import math
from typing import Dict, Tuple, List


class AtmosphericLayer:
    def __init__(
        self,
        name: str,
        base_altitude_m: float,
        top_altitude_m: float,
        base_temp_k: float,
        lapse_rate_k_per_m: float,
        base_pressure_pa: float
    ):
        self.name = name
        self.base_altitude_m = base_altitude_m
        self.top_altitude_m = top_altitude_m
        self.base_temp_k = base_temp_k
        self.lapse_rate_k_per_m = lapse_rate_k_per_m
        self.base_pressure_pa = base_pressure_pa


class AtmosphereModel:
    R_SPECIFIC = 287.058  # Specific gas constant for dry air, J/(kg·K)
    G0 = 9.80665          # Gravitational acceleration at sea level, m/s^2
    EARTH_RADIUS_M = 6356766.0  # Effective earth radius for geopotential conversion
    GAMMA = 1.4           # Heat capacity ratio for air

    def __init__(self):
        # Define 7 standard atmospheric layers up to 86km
        self.layers = [
            AtmosphericLayer("Troposphere", 0.0, 11000.0, 288.15, -0.0065, 101325.0),
            AtmosphericLayer("Tropopause", 11000.0, 20000.0, 216.65, 0.0, 22632.1),
            AtmosphericLayer("Stratosphere Low", 20000.0, 32000.0, 216.65, 0.0010, 5474.89),
            AtmosphericLayer("Stratosphere High", 32000.0, 47000.0, 228.65, 0.0028, 868.019),
            AtmosphericLayer("Stratopause", 47000.0, 51000.0, 270.65, 0.0, 110.906),
            AtmosphericLayer("Mesosphere Low", 51000.0, 71000.0, 270.65, -0.0028, 66.9388),
            AtmosphericLayer("Mesosphere High", 71000.0, 84852.0, 214.65, -0.0020, 3.95642)
        ]

    def geometric_to_geopotential(self, altitude_m: float) -> float:
        """Converts geometric altitude to geopotential altitude."""
        return (self.EARTH_RADIUS_M * altitude_m) / (self.EARTH_RADIUS_M + altitude_m)

    def get_layer(self, h_geo: float) -> AtmosphericLayer:
        for layer in self.layers:
            if layer.base_altitude_m <= h_geo <= layer.top_altitude_m:
                return layer
        if h_geo > self.layers[-1].top_altitude_m:
            return self.layers[-1]
        return self.layers[0]

    def calculate_state(self, altitude_m: float) -> Dict[str, float]:
        h = self.geometric_to_geopotential(altitude_m)
        layer = self.get_layer(h)

        h0 = layer.base_altitude_m
        t0 = layer.base_temp_k
        L = layer.lapse_rate_k_per_m
        p0 = layer.base_pressure_pa

        dh = h - h0

        if abs(L) < 1e-10:  # Isothermal layer
            temperature = t0
            pressure = p0 * math.exp((-self.G0 * dh) / (self.R_SPECIFIC * t0))
        else:  # Gradient layer
            temperature = t0 + L * dh
            pressure = p0 * ((temperature / t0) ** (-self.G0 / (L * self.R_SPECIFIC)))

        density = pressure / (self.R_SPECIFIC * temperature)
        speed_of_sound = math.sqrt(self.GAMMA * self.R_SPECIFIC * temperature)

        return {
            "altitude_m": altitude_m,
            "geopotential_m": h,
            "layer_name": layer.name,
            "temperature_k": round(temperature, 2),
            "temperature_c": round(temperature - 273.15, 2),
            "pressure_pa": round(pressure, 2),
            "density_kg_m3": round(density, 5),
            "speed_of_sound_m_s": round(speed_of_sound, 2)
        }

    def simulate_vertical_profile(self, max_alt_km: int = 80, step_km: float = 2.0) -> List[Dict[str, float]]:
        results = []
        alt_m = 0.0
        while alt_m <= max_alt_km * 1000:
            state = self.calculate_state(alt_m)
            results.append(state)
            alt_m += step_km * 1000
        return results


class AtmosphericReportGenerator:
    @staticmethod
    def print_ascii_table(data: List[Dict[str, float]]) -> None:
        header = f"| {'Alt (km)':^8} | {'Layer':^18} | {'Temp (°C)':^10} | {'Press (Pa)':^12} | {'Density (kg/m³)':^16} | {'Speed Sound':^11} |"
        sep = "-" * len(header)
        print(sep)
        print(header)
        print(sep)

        for row in data:
            alt_km = row["altitude_m"] / 1000.0
            print(
                f"| {alt_km:8.1f} | "
                f"{row['layer_name']:<18} | "
                f"{row['temperature_c']:10.2f} | "
                f"{row['pressure_pa']:12.2f} | "
                f"{row['density_kg_m3']:16.5f} | "
                f"{row['speed_of_sound_m_s']:11.2f} |"
            )
        print(sep)


def run_tests():
    model = AtmosphereModel()
    sea_level = model.calculate_state(0)
    assert sea_level["temperature_k"] == 288.15
    assert sea_level["pressure_pa"] == 101325.0
    print("Self-check passed: Sea level properties validated.")


def main():
    print("=========================================================")
    print("       EARTH ATMOSPHERE VERTICAL PROFILE SIMULATOR       ")
    print("=========================================================")
    run_tests()
    model = AtmosphereModel()
    profile = model.simulate_vertical_profile(max_alt_km=80, step_km=4.0)
    AtmosphericReportGenerator.print_ascii_table(profile)

    # Additional calculations for specific key Earth altitudes
    milestones = [
        ("Mt. Everest Peak", 8848.86),
        ("Commercial Flight Level", 11000.0),
        ("Stratospheric Balloon", 39000.0),
        ("Mesosphere Boundary", 80000.0)
    ]

    print("\n--- Key Milestone Analysis ---")
    for name, alt in milestones:
        res = model.calculate_state(alt)
        print(f"[{name} @ {alt:.0f}m]: Temp = {res['temperature_c']}°C, Pressure = {res['pressure_pa']} Pa, Density = {res['density_kg_m3']} kg/m³")


if __name__ == "__main__":
    main()