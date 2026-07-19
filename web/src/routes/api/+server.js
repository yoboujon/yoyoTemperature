import { json, error } from '@sveltejs/kit';
import { API_IP, API_PORT } from '$env/static/private';

const BASE_URL = `http://${API_IP}:${API_PORT}/measurements`

export async function GET({ url, fetch }) {

	const from = url.searchParams.get('from');
	const to = url.searchParams.get('to');
	if (!from || !to) {
		throw error(400, 'Missing required params: from, to');
	}
	const apiUrl = new URL(BASE_URL);
	apiUrl.searchParams.set('method', 'get');
	apiUrl.searchParams.set('format', 'json');
	apiUrl.searchParams.set('from', from);
	apiUrl.searchParams.set('to', to);

	let res;
	try {
		res = await fetch(apiUrl);
	} catch (err) {
		throw error(502, `Failed to reach Infoclimat: ${err.message}`);
	}
	if (!res.ok) {
		throw error(res.status, `Infoclimat returned ${res.status}`);
	}
	const data = await res.json();
	return json(data);
}
