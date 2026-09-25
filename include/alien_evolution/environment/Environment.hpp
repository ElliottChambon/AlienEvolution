#pragma once

namespace ae
{

    struct Environment
    {
        // Gravitational acceleration in m/s^2.
        double gravity = 9.80665;

        // Dimensionless environmental resource availability.
        // 1.0 represents the V0.1 reference environment.
        double resourceAvailability = 1.0;
    };

} // namespace ae