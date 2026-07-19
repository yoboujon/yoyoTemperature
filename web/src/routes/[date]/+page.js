export async function load({ fetch, params }) {
	const date = params.date;

	const from = `${date}T04:00:00`;
	const d = new Date(date);
	d.setDate(d.getDate() + 1);
	const nextDate = d.toISOString().slice(0, 10);
	const to = `${nextDate}T04:00:00`;

	const res = await fetch(`/api?from=${from}&to=${to}`);
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
	};
}