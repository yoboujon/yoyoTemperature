import { json, error } from '@sveltejs/kit';
import { API_IP, API_PORT } from '$env/static/private';

const BASE_URL = `http://${API_IP}:${API_PORT}/measurements/month`

export async function GET({ url, fetch }) {

	const date = url.searchParams.get('date');
	if (!date) {
		throw error(400, 'Missing required params: date');
	}

	const d = new Date(date);
	const str_date = String(d.getMonth()+1).padStart(2, '0') + "-" + d.getFullYear();
	let max_res;
	let min_res;

	try {
		max_res = await fetch(`${BASE_URL}/${str_date}/max`);
		min_res = await fetch(`${BASE_URL}/${str_date}/min`);
	} catch (err) {
		throw error(502, `Failed to reach Infoclimat: ${err.message}`);
	}

	if (!max_res.ok) {
		throw error(max_res.status, `Infoclimat returned ${max_res.status}`);
	}
	if (!min_res.ok) {
		throw error(min_res.status, `Infoclimat returned ${min_res.status}`);
	}

	const max_data = await max_res.json();
	const min_data = await min_res.json();
	return json({
		max: max_data,
		min: min_data,
	});
}
