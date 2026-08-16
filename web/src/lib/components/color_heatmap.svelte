<script>
	import { colorAtTemp } from "$lib/gradient.js";
    
    let { value } = $props();
</script>

<div
    class="color-heatmap"
    style={"background-color: rgb(" + colorAtTemp(value) + ")"}
></div>
