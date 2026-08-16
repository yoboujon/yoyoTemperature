const tempGradient = [
    [0, [174, 226, 255]],
    [15, [67, 130, 223]],
    [20, [47, 160, 132]],
    [25, [255, 200, 30]],
    [28, [232, 127, 36]],
    [31, [170, 28, 65]],
    [40, [94, 36, 78]],
];

function interpolateColor(c1, c2, t) {
    return c1.map((v, i) => Math.round(v + (c2[i] - v) * t));
}

export function colorAtTemp(temp) {
    for (let i = 0; i < tempGradient.length - 1; i++) {
        const [t1, c1] = tempGradient[i];
        const [t2, c2] = tempGradient[i + 1];

        if (temp >= t1 && temp <= t2) {
            const ratio = (temp - t1) / (t2 - t1);
            return interpolateColor(c1, c2, ratio);
        }
    }

    if (temp <= tempGradient[0][0]) return tempGradient[0][1];
    return tempGradient[tempGradient.length - 1][1];
}

export function get_stops(min, max) {
    const visibleStops = [];
    visibleStops.push({
        offset: 1,
        color: `rgb(${colorAtTemp(min).join(",")})`,
    });
    for (const [t, rgb] of tempGradient) {
        if (t > min && t < max) {
            visibleStops.push({
                offset: 1 - (t - min) / (max - min),
                color: `rgb(${rgb.join(",")})`,
            });
        }
    }
    visibleStops.push({
        offset: 0,
        color: `rgb(${colorAtTemp(max).join(",")})`,
    });
    return visibleStops;
}