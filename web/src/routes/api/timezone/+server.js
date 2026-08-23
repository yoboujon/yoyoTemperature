import { json, error } from '@sveltejs/kit';
import { API_IP, API_PORT } from '$env/static/private';

const BASE_URL = `http://${API_IP}:${API_PORT}/timezone`

export async function GET({ url, fetch }) {
	let res;
    try {
		res = await fetch(BASE_URL);
	} catch (err) {
		throw error(502, `Failed to reach API: ${err.message}`);
	}
	if (!res.ok) {
		throw error(res.status, `API returned ${max_res.status}`);
	}

	return await res;
}
