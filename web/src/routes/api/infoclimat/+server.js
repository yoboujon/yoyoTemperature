// &start=2026-07-16&end=2026-07-18&token=

import { json, error } from '@sveltejs/kit';
import { INFOCLIMAT_TOKEN } from '$env/static/private';

const BASE_URL = "https://www.infoclimat.fr/opendata/";
const CACHE_DURATION_MS = 5 * 60 * 1000;
let cachedData = null;
let lastFetchTime = null;

export async function GET({ url, fetch }) {
    const now = new Date();
    if (cachedData && lastFetchTime && (now - lastFetchTime) < CACHE_DURATION_MS) {
        return json(cachedData);
    }

    const date_str = now.toISOString().slice(0, 10);
    const apiUrl = new URL(BASE_URL);
    apiUrl.searchParams.set('version', 2);
    apiUrl.searchParams.set('method', 'get');
    apiUrl.searchParams.set('format', 'json');
    apiUrl.searchParams.set('stations[]', '000HF');
    apiUrl.searchParams.set('token', INFOCLIMAT_TOKEN);
    apiUrl.searchParams.set('start', date_str);
    apiUrl.searchParams.set('end', date_str);

    let res;
    try {
        res = await fetch(apiUrl);
    } catch (err) {
        if (cachedData) {
            return json(cachedData);
        }
        throw error(502, `Failed to reach Infoclimat: ${err.message}`);
    }

    if (!res.ok) {
        if (cachedData) {
            return json(cachedData);
        }
        throw error(res.status, `Infoclimat returned ${res.status}`);
    }

    const data = await res.json();

    cachedData = data;
    lastFetchTime = now;

    return json(data);
}
