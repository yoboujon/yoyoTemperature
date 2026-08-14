<script>
	import "$lib/css/base.css";
	import "$lib/css/button.css";
	import "$lib/css/temperature.css";

	import TemperatureWidget from "$lib/components/temperature_widget.svelte";
	import ChartWidget from "$lib/components/chart_widget.svelte";
	import SvgIcon from "@jamescoyle/svelte-icon";
	import { mdiHomeClock } from "@mdi/js";

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

		const temps_epoch = data.json.day.measurements.map((m) => [
			m.epoch * 1000,
			m.temperature,
		]);
		const temps = data.json.day.measurements.map((m) => [m.temperature]);
		const nextCand = shiftDate(data.date, 1);
		return {
			temperatures: temps_epoch,
			min: Math.floor(data.extremum.min.temperature),
			max: Math.ceil(data.extremum.max.temperature),
			actual_max: temps.length ? Math.ceil(Math.max(...temps)) : null,
			temperatures_now: {
				outdoor: data.infoclimat.hourly["000HF"].at(-1).temperature,
				indoor: data.json.now.temperature,
			},
			exists: data.json.exists.state,
			max_min: data.json.max_min.measurements,
		};
	});
</script>

{#if data.error}
	<p class="error">{data.error}</p>
{:else}
	{#await pageData}
		<p>Loading temperatures...</p>
	{:then value}
		<div class="flex-row">
			<div class="flex-col section main-widgets chart">
				<h2>Indoor Temperatures</h2>
				<ChartWidget
					temperatures={value.temperatures}
					max={value.max}
					min={value.min}
					actualMax={value.actual_max}
				/>
				<div class="flex center">
					{#if value.exists}
						<a class="button" href="/{prev}"
							><span>&lt; {prev}</span></a
						>
					{/if}
					{#if !(new Date(data.date).toDateString() === new Date().toDateString())}
						<a
							class="button"
							href="/{new Date().toISOString().slice(0, 10)}"
						>
							<span>
								<SvgIcon type="mdi" path={mdiHomeClock} />
							</span>
						</a>
					{/if}
					{#if next != null}
						<a class="button" href="/{next}"
							><span>{next} &gt;</span></a
						>
					{/if}
				</div>
			</div>

			<div class="flex-col flex-fill section align-center main-widgets">
				<h2>Real-Time Temperature</h2>
				<div class="flex-fill center">
					<TemperatureWidget value={value.temperatures_now} />
				</div>
			</div>
		</div>
		<table>
			<thead>
				<tr>
					<th>Date</th>
					<th>T°C Min.</th>
					<th>T°C Max.</th>
				</tr>
			</thead>
			<tbody>
				{#each value.max_min as mm}
					<tr>
						<th>{new Date(mm.date).toISOString().slice(0, 10)}</th>
						<td>{mm.min.toFixed(1)}°C</td>
						<td>{mm.max.toFixed(1)}°C</td>
					</tr>
				{/each}
			</tbody>
		</table>
	{:catch error}
		<p>Failed to load temperature: {error.message}</p>
	{/await}
{/if}
