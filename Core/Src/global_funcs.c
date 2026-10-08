//
// Created by sergey on 10/1/26.
//

float clamp(
    float value,
    float min,
    float max
)
{
    if (value < min)
    {
        return min;
    }

    if (value > max)
    {
        return max;
    }

    return value;
}
