import { redirect } from '@sveltejs/kit';

export function load() {
	const today = new Date();
	const date = today.toISOString().slice(0, 10);

	throw redirect(307, `/${date}`);
}