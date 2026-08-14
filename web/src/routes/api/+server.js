import { json, error } from '@sveltejs/kit';
import { API_IP, API_PORT } from '$env/static/private';

const BASE_URL = `http://${API_IP}:${API_PORT}/`

export async function GET({ url, fetch }) {

	const from = url.searchParams.get('from');
	const to = url.searchParams.get('to');
	if (!from || !to) {
		throw error(400, 'Missing required params: from, to');
	}

	const d = new Date(from);
	const str_date = String(d.getDate()).padStart(2, '0') + "-" + String(d.getMonth()+1).padStart(2, '0') + "-" + d.getFullYear();

	const measurementsUrl = new URL("measurements", BASE_URL);
	measurementsUrl.searchParams.set('from', from);
	measurementsUrl.searchParams.set('to', to);
	const existsUrl = new URL(`measurements/exists/${str_date}`, BASE_URL);

	let res, res_now, res_exists;
	try {
		res = await fetch(measurementsUrl);
		res_now = await fetch(BASE_URL);
		res_exists = await fetch(existsUrl);
	} catch (err) {
		throw error(502, `Failed to reach API: ${err.message}`);
	}
	if (!res.ok) {
		throw error(res.status, `API returned ${res.status}`);
	} else if (!res_now.ok) {
		throw error(res_now.status, `API returned ${res_now.status}`);
	} else if (!res_exists.ok) {
		throw error(res_exists.status, `API returned ${res_exists.status}`);
	}

	const data = await res.json();
	const data_now = await res_now.json();
	const data_exists = await res_exists.json();

	return json({
		day: data,
		now: data_now,
		exists: data_exists
	});
}
