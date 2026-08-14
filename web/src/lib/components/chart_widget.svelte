<script>
    import { onMount } from "svelte";
    import * as echarts from "echarts";
    import { get_stops } from "$lib/gradient.js";

    let { temperatures, max, min, actualMax } = $props();
	let chartDiv = $state(0);
	let chart;

    $effect(() => {
        // Animating echart
        if (!chart) {
            chart = echarts.init(chartDiv);
        }
        chart.setOption({
            animation: false,
            tooltip: {
                trigger: "axis",
                position: (pt) => [pt[0], "10%"],
            },
            title: {
                left: "center",
                text: "Indoor Temperature",
            },
            toolbox: {
                feature: {
                    dataZoom: {
                        yAxisIndex: "none",
                    },
                    restore: {},
                    saveAsImage: {},
                },
            },
            xAxis: {
                type: "time",
                axisLabel: {
                    formatter(value) {
                        return new Date(value).toLocaleTimeString("fr-FR", {
                            hour: "2-digit",
                            minute: "2-digit",
                            hour12: false,
                        });
                    },
                },
            },

            yAxis: {
                type: "value",
                name: "°C",
                min: min,
                max: max,
            },
            series: [
                {
                    name: "Temperature",
                    type: "line",
                    symbol: "none",
                    sampling: "lttb",
                    areaStyle: {
                        color: new echarts.graphic.LinearGradient(
                            0,
                            0,
                            0,
                            1,
                            get_stops(min, actualMax),
                        ),
                    },
                    data: temperatures
                },
            ],
            visualMap: {
                show: false,
                dimension: 1,
                pieces: [
                    { lte: 0, color: "rgb(174,226,255)" },
                    { gt: 15, lte: 20, color: "rgb(67,130,223)" },
                    { gt: 20, lte: 25, color: "rgb(47,160,132)" },
                    { gt: 25, lte: 28, color: "rgb(255,200,30)" },
                    { gt: 28, lte: 31, color: "rgb(232,127,36)" },
                    { gt: 31, lte: 40, color: "rgb(170,28,65)" },
                    { gt: 40, color: "rgb(94,36,78)" },
                ],
            },
        });
    });
</script>

<div bind:this={chartDiv} style="width: 80dvw; height: 50dvh;"></div>
