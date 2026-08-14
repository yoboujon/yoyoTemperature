import { json, error } from '@sveltejs/kit';
import { API_IP, API_PORT } from '$env/static/private';

const BASE_URL = `http://${API_IP}:${API_PORT}/`

export async function GET({ url, fetch }) {

	const from = url.searchParams.get('from');
	const to = url.searchParams.get('to');
	if (!from || !to) {
		throw error(400, 'Missing required params: from, to');
	}

	const apiUrl = new URL("measurements", BASE_URL);
	apiUrl.searchParams.set('method', 'get');
	apiUrl.searchParams.set('format', 'json');
	apiUrl.searchParams.set('from', from);
	apiUrl.searchParams.set('to', to);

	let res, res_now;
	try {
		res = await fetch(apiUrl);
		res_now = await fetch(BASE_URL);
	} catch (err) {
		throw error(502, `Failed to reach API: ${err.message}`);
	}
	if (!res.ok) {
		throw error(res.status, `API returned ${res.status}`);
	} else if (!res_now.ok) {
		throw error(res.status, `API returned ${res.status}`);
	}
	const data = await res.json();
	const data_now = await res_now.json();
	return json({
		day: data,
		now: data_now
	});
}
