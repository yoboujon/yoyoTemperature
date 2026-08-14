<script>
	import "$lib/css/base.css";
	import "$lib/css/button.css";
	import "$lib/css/temperature.css";

	import TemperatureWidget from "$lib/components/temperature_widget.svelte";
	import ChartWidget from "$lib/components/chart_widget.svelte";
	import * as echarts from "echarts";
	import { get_stops } from "$lib/gradient.js";

	function shiftDate(date, days) {
		const d = new Date(date);
		d.setDate(d.getDate() + days);
		return d.toISOString().slice(0, 10);
	}

	// Date
	const todayStr = new Date().toISOString().slice(0, 10);
	const prev = $derived(!data.error ? shiftDate(data.date, -1) : null);
	const nextCandidate = $derived(
		!data.error ? shiftDate(data.date, 1) : null,
	);
	const next = $derived(
		nextCandidate && nextCandidate > todayStr ? null : nextCandidate,
	);

	// API calls
	let { data } = $props();
	const pageData = $derived.by(() => {
		if (data.error) return null;

		const temps_epoch = data.json.measurements.map((m) => [
			m.epoch * 1000,
			m.temperature,
		]);
		const temps = data.json.measurements.map((m) => [m.temperature]);
		const nextCand = shiftDate(data.date, 1);
		return {
			temperatures: temps_epoch,
			min: Math.floor(data.extremum.min.temperature),
			max: Math.ceil(data.extremum.max.temperature),
			actual_max: temps.length ? Math.ceil(Math.max(...temps)) : null,
			temperatures_now: {
				outdoor: data.infoclimat.hourly["000HF"].at(-1).temperature,
				indoor: temps.at(-1),
			},
		};
	});
</script>

{#if data.error}
	<p class="error">{data.error}</p>
{:else}
	<div class="flex-col">
		<div class="flex-row">
			{#await pageData}
				<p>Loading temperatures...</p>
			{:then value}
				<ChartWidget
					temperatures={value.temperatures}
					max={value.max}
					min={value.min}
					actualMax={value.actual_max}
				/>
				<TemperatureWidget value={value.temperatures_now} />
			{:catch error}
				<p>Failed to load temperature: {error.message}</p>
			{/await}
		</div>

		<div class="flex center">
			<a class="button" href="/{prev}"><span>&lt; {prev}</span></a>
			{#if next != null}
				<a class="button" href="/{next}"><span>{next} &gt;</span></a>
			{/if}
		</div>
	</div>
{/if}
