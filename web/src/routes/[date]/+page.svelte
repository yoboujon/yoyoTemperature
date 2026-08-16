<script>
	import { goto } from "$app/navigation";

	import "$lib/css/base.css";
	import "$lib/css/button.css";
	import "$lib/css/temperature.css";

	import TemperatureWidget from "$lib/components/temperature_widget.svelte";
	import ChartWidget from "$lib/components/chart_widget.svelte";
	import ColorHeatmap from "$lib/components/color_heatmap.svelte";

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
			min: data.extremum.min,
			max: data.extremum.max,
			actual_max: temps.length ? Math.ceil(Math.max(...temps)) : null,
			temperatures_now: {
				outdoor: data.infoclimat.hourly["000HF"].at(-1).temperature,
				indoor: data.json.now.temperature,
			},
			exists: data.json.exists.state,
			max_min: data.json.max_min.measurements,
		};
	});

	// Source - https://stackoverflow.com/a/8016205 License - CC BY-SA 3.0
	function getEpochDate(d) {
		var date = new Date(0);
		date.setUTCSeconds(d);
		return date;
	}
	// Functions helper
	function navigate(path, disabled) {
		if (disabled) return;
		goto(`/${path}`, { noScroll: true });
	}
	function formatDate(d, show_hour = true) {
		const pad = (n) => String(n).padStart(2, "0");
		const hour = `@ ${d.getHours()}:${pad(d.getMinutes())}`;
		return `${pad(d.getUTCDate())}/${pad(d.getUTCMonth() + 1)}/${d.getUTCFullYear()} ${show_hour ? hour : ""}`;
	}
	function formatHour(d) {
		const pad = (n) => String(n).padStart(2, "0");
		return `${d.getHours()}h${pad(d.getMinutes())}`;
	}
	function isEqualDate(epoch_1, epoch_2) {
		return (
			getEpochDate(epoch_1).toDateString() ==
			getEpochDate(epoch_2).toDateString()
		);
	}

	function show(epoch_1, epoch_2) {
		return `${getEpochDate(epoch_1).toDateString()} == ${getEpochDate(epoch_2).toDateString()}`
	}
</script>

{#if data.error}
	<p class="error">{data.error}</p>
{:else}
	{#await pageData}
		<p>Loading temperatures...</p>
	{:then value}
		<div class="flex-col-mobile">
			<div class="flex-col section main-widgets chart">
				<h2>Indoor Temperatures</h2>
				<ChartWidget
					temperatures={value.temperatures}
					max={Math.ceil(value.max.temperature)}
					min={Math.floor(value.min.temperature)}
					actualMax={value.actual_max}
				/>
				<div class="flex center">
					<button
						class={value.exists ? "button" : "button-disabled"}
						onclick={() =>
							navigate(
								value.exists ? prev : data.date,
								!value.exists,
							)}
					>
						<span>&lt; {prev}</span>
					</button>

					<button
						class={new Date(data.date).toDateString() ===
						new Date().toDateString()
							? "button-disabled"
							: "button"}
						onclick={() =>
							navigate(
								new Date().toISOString().slice(0, 10),
								new Date(data.date).toDateString() ===
									new Date().toDateString(),
							)}
					>
						<SvgIcon type="mdi" path={mdiHomeClock} />
					</button>

					<button
						class={next != null ? "button" : "button-disabled"}
						onclick={() =>
							navigate(
								next != null ? next : nextCandidate,
								next == null,
							)}
					>
						<span>{next != null ? next : nextCandidate} &gt;</span>
					</button>
				</div>
			</div>

			<div
				class="flex-col flex-fill section align-center main-widgets real-time"
			>
				<h2>Real-Time Temperature</h2>
				<div class="flex-fill center">
					<TemperatureWidget value={value.temperatures_now} />
				</div>
			</div>
		</div>
		<div style="height:5rem;"></div>
		<div class="w-100 center flex-col">
			<div style="width: 80dvw;">
				<table class="maximini">
					<tbody class="maximini">
						<tr>
							<th>Maxi/Mini du mois<br /><i>(TNN, TXX)</i></th>
							<td class="min">
								<span>
									<b>{value.min.temperature.toFixed(1)}°C</b>
								</span>
								<br />
								<span
									><i>
										the {formatDate(
											new Date(value.min.date),
										)}
									</i></span
								>
							</td>
							<td class="max">
								<span>
									<b>{value.max.temperature.toFixed(1)}°C</b>
								</span>
								<br />
								<span
									><i>
										the {formatDate(
											new Date(value.max.date),
										)}
									</i></span
								>
							</td>
						</tr>
					</tbody>
				</table>
			</div>
			<div style="height:2rem;"></div>
			<div style="width: 80dvw;">
				<table>
					<thead>
						<tr>
							<th>Date</th>
							<th>T°C Min.</th>
							<th>T°C Max.</th>
						</tr>
					</thead>
					<tbody class="table-temperatures">
						{#each value.max_min as mm}
							<tr>
								<th class="date">
									{formatDate(
										getEpochDate(mm.max_epoch),
										false,
									)}
								</th>
								<td
									class={isEqualDate(
										mm.min_epoch,
										value.min.epoch,
									)
										? "min"
										: ""}
								>
									<ColorHeatmap value={mm.min} />
									<span><b>{mm.min.toFixed(1)}</b>°C</span>
									<br />
									<span
										><i
											>{formatHour(
												getEpochDate(mm.min_epoch),
											)}</i
										></span
									>
									<!--<br />
									<span>{show(mm.min_epoch,value.min.epoch)}</span>-->
								</td>
								<td
									class={isEqualDate(
										mm.max_epoch,
										value.max.epoch,
									)
										? "max"
										: ""}
								>
									<ColorHeatmap value={mm.max} />
									<span><b>{mm.max.toFixed(1)}</b>°C</span>
									<br />
									<span
										><i
											>{formatHour(
												getEpochDate(mm.max_epoch),
											)}</i
										></span
									>
									<!--<span>{show(mm.max_epoch,value.max.epoch)}</span>-->
								</td>
							</tr>
						{/each}
					</tbody>
				</table>
			</div>
		</div>
		<div style="height:5rem;"></div>
	{:catch error}
		<p>Failed to load temperature: {error.message}</p>
	{/await}
{/if}
