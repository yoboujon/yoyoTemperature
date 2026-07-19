<script>
	import "$lib/css/base.css";
	import "$lib/css/button.css";
	import "$lib/css/temperature.css";

	import { onMount, onDestroy } from "svelte";
	import * as echarts from "echarts";
	import { get_stops } from "$lib/gradient.js";
	import SvgIcon from "@jamescoyle/svelte-icon";
	import { mdiHomeThermometer, mdiSunThermometer } from "@mdi/js";
	import { animate, utils } from "animejs";

	function shiftDate(date, days) {
		const d = new Date(date);
		d.setDate(d.getDate() + days);
		return d.toISOString().slice(0, 10);
	}

	const today = new Date();
	let chartDiv = $state(0);
	let chart;
	let { data } = $props();
	let infoclimat_last_temp = $state(null);
	let indoor_last_temp = $state(null);
	const temperatures_now = $state({ outdoor: 0.0, indoor: 0.0 });

	const prev = $derived(shiftDate(data.date, -1));
	const todayStr = today.toISOString().slice(0, 10);
	const nextCandidate = $derived(shiftDate(data.date, 1));
	const next = $derived(nextCandidate > todayStr ? null : nextCandidate);

	$effect(() => {
		const temperatures = data.json.measurements.map((m) => m.temperature);
		const min = Math.floor(data.extremum.min.temperature);
		const max = Math.ceil(data.extremum.max.temperature);
		const actual_max = Math.ceil(
			temperatures.reduce((a, b) => Math.max(a, b), -Infinity),
		);

		// Animating "Now temperatures"
		const animation = animate(temperatures_now, {
			outdoor: data.infoclimat.hourly["000HF"].at(-1).temperature,
			indoor: temperatures.at(-1),
			modifier: utils.round(1),
			duration: 1000,
			ease: 'outExpo'
		});

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
							get_stops(min, actual_max),
						),
					},
					data: data.json.measurements.map((m) => [
						m.epoch * 1000,
						m.temperature,
					]),
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

{#if data.error}
	<p class="error">{data.error}</p>
{:else}
	<div class="flex-col">
		<div class="flex-row">
			<div
				bind:this={chartDiv}
				style="width: 80dvw; height: 50dvh;"
			></div>
			<div class="temperature flex-col center">
				<div class="flex-row">
					<SvgIcon type="mdi" path={mdiSunThermometer}></SvgIcon>
					<span><b>{temperatures_now.outdoor.toFixed(1)}°C</b></span>
				</div>
				<div class="flex-row">
					<SvgIcon type="mdi" path={mdiHomeThermometer}></SvgIcon>
					<span>{temperatures_now.indoor.toFixed(1)}°C</span>
				</div>
			</div>
		</div>

		<div class="flex center">
			<a class="button" href="/{prev}"><span>&lt; {prev}</span></a>
			{#if next != null}
				<a class="button" href="/{next}"><span>{next} &gt;</span></a>
			{/if}
		</div>
	</div>
{/if}
