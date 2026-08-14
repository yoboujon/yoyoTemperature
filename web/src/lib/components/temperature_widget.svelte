<script>
    import { onMount } from "svelte";
    import SvgIcon from "@jamescoyle/svelte-icon";
    import { mdiHomeThermometer, mdiSunThermometer } from "@mdi/js";
    import { animate, utils } from "animejs";

    let { value } = $props();
    const temperatures_then = $state({ outdoor: 0.0, indoor: 0.0 });

    $effect(() => {
        if (value?.outdoor == null || value?.indoor == null) return;
        const animation = animate(temperatures_then, {
            outdoor: value.outdoor,
            indoor: value.indoor,
            modifier: utils.round(1),
            duration: 1000,
            ease: "outExpo",
        });
    });
</script>

<div class="temperature flex-col center">
    <div class="flex-row">
        <SvgIcon type="mdi" path={mdiSunThermometer}></SvgIcon>
        <span><b>{temperatures_then.outdoor.toFixed(1)}°C</b></span>
    </div>
    <div class="flex-row">
        <SvgIcon type="mdi" path={mdiHomeThermometer}></SvgIcon>
        <span>{temperatures_then.indoor.toFixed(1)}°C</span>
    </div>
</div>
