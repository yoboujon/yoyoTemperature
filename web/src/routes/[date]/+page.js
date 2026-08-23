function convertTZ(date, tzString) {
    return new Date((typeof date === "string" ? new Date(date) : date).toLocaleString("en-US", {timeZone: tzString}));   
}

function formatDate(date) {
    const pad = (n) => String(n).padStart(2, "0");
    return `${date.getFullYear()}-${pad(date.getMonth() + 1)}-${pad(date.getDate())}T${pad(date.getHours())}:${pad(date.getMinutes())}:${pad(date.getSeconds())}`;
}

function shiftDate(date, days) {
	const d = new Date(date);
	d.setDate(d.getDate() + days);
	return d.toISOString().slice(0, 10);
}

export async function load({ fetch, params }) {
	// Timezone first
	const res_tz = await fetch("/api/timezone");
	if (!res_tz.ok) {
		return {
			error: `Request failed: Error (Timezone) ${res_tz.status}`
		};
	}
	const tz = await res_tz.json();
	
	const timezone = tz.timezone;
	const date = params.date;

	const today = new Date(date);
	const next = shiftDate(today, 1);

	const res = await fetch(`/api?from=${formatDate(convertTZ(today, timezone))}&to=${formatDate(convertTZ(next, timezone))}`);
	const res_extremum = await fetch(`/api/month?date=${date}`);
	const res_infoclimat = await fetch("/api/infoclimat");

	if (!res.ok) {
		return {
			error: `Request failed: Error (Data) ${res.status}`
		};
	}
	if (!res_extremum.ok) {
		return {
			error: `Request failed: Error (Extremum) ${res.status}`
		};
	}
	if (!res_infoclimat.ok) {
		return {
			error: `Request failed: Error (Infoclimat) ${res.status}`
		};
	}

	return {
		date,
		json: await res.json(),
		extremum: await res_extremum.json(),
		infoclimat: await res_infoclimat.json(),
		timezone: timezone
	};
}