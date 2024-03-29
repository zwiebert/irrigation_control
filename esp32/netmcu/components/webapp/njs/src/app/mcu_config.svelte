<script>
  "use strict";
  import { _ } from "../services/i18n";
  import { McuConfig, McuConfigKeys } from "../store/mcu_config.js";
  import * as appDebug from "../app/app_debug.js";
  import * as httpFetch from "../app/fetch.js";
  import { req_mcuRestart } from "../app/misc.js";
  import { onMount, onDestroy } from "svelte";
  import { ReloadProgress } from "../store/app_state.js";

  import McuConfigGpio from "../components/mcu_config/gpio.svelte";
  import McuConfigNetwork from "../components/mcu_config/network.svelte";
  import McuConfigLanPhy from "../components/mcu_config/lan_phy.svelte";
  import McuConfigNumber from "../components/mcu_config/number.svelte";
  import McuConfigEnable from "../components/mcu_config/enable.svelte";

  let on_destroy = [];
  onMount(() => {
    httpFetch.http_fetchByMask(httpFetch.FETCH_CONFIG);
  });
  onDestroy(() => {
    for (const fn of on_destroy) {
      fn();
    }
  });

  $: mcuConfigKeys = $McuConfigKeys;
  $: mcuConfig = $McuConfig;

  export function reload_config() {
    // eslint-disable-next-line no-unused-vars
    Object.keys($McuConfig).forEach(function (key, idx) {
      let el = document.getElementById("cfg_" + key);

      switch (el.type) {
        case "checkbox":
          el.checked = $McuConfig[key] !== 0;
          break;
        default:
          el.value = $McuConfig[key];
      }
    });
  }

  function hClick_Reload() {
    reload_config();
    httpFetch.http_fetchByMask(httpFetch.FETCH_CONFIG);
  }

  function hClick_Save() {
    mcuConfig_fromHtml_toMcu();
  }

  function mcuConfig_fromHtml_toMcu() {
    const cfg = $McuConfig;

    let new_cfg = {};
    let has_changed = false;
    // eslint-disable-next-line no-unused-vars
    Object.keys(cfg).forEach(function (key, idx) {
      let new_val = 0;
      let el = document.getElementById("cfg_" + key);
      appDebug.dbLog("key: " + key);

      switch (el.type) {
        case "checkbox":
          new_val = el.checked ? 1 : 0;
          break;
        case "number":
          new_val = el.valueAsNumber;
          break;
        default:
          new_val = el.value;
      }
      let old_val = cfg[key];
      if (new_val !== old_val) {
        new_cfg[key] = new_val;
        has_changed = true;
        appDebug.dbLog(key);
      }
    });

    if (has_changed) {
      new_cfg.all = "?";
      let url = "/cmd.json";
      httpFetch.http_postRequest(url, { config: new_cfg });
    }
  }
</script>

<style lang="scss">

</style>

<div id="configdiv" class="top-div">
  <div class="config" id="config-div">
    <table id="cfg_table_id" class="conf-table top_table">
      {#each mcuConfigKeys as name, i}
        <tr class={i % 2 ? 'bg-yellow-200' : 'bg-yellow-300'}>

          {#if name !== 'gm-used'}
            <td>
              <label class="config-label">{name}</label>
            </td>
          {/if}

          {#if name.endsWith('-enable') || name === 'stm32-bootgpio-inv'}
            <td>
              <McuConfigEnable {name} value={mcuConfig[name]} />
            </td>
          {:else if name === 'latitude'}
            <td>
              <McuConfigNumber
                {name}
                value={mcuConfig[name]}
                min="-90"
                max="90"
                step="0.01" />
            </td>
          {:else if name === 'longitude'}
            <td>
              <McuConfigNumber
                {name}
                value={mcuConfig[name]}
                min="-180"
                max="180"
                step="0.01" />
            </td>
          {:else if name === 'rf-rx-pin' || name === 'set-button-pin'}
            <td>
              <McuConfigNumber
                {name}
                value={mcuConfig[name]}
                min="-1"
                max="39" />
            </td>
          {:else if name === 'rf-tx-pin'}
            <td>
              <McuConfigNumber
                {name}
                value={mcuConfig[name]}
                min="-1"
                max="33" />
            </td>
          {:else if name === 'verbose'}
            <td>
              <McuConfigNumber {name} value={mcuConfig[name]} min="0" max="5" />
            </td>
          {:else if name === 'network'}
            <td>
              <McuConfigNetwork {name} value={mcuConfig[name]} />
            </td>
          {:else if name === 'lan-phy'}
            <td>
              <McuConfigLanPhy {name} value={mcuConfig[name]} />
            </td>
          {:else if name === 'lan-pwr-gpio'}
            <td>
              <McuConfigNumber
                {name}
                value={mcuConfig[name]}
                min="-1"
                max="36" />
            </td>
          {:else if name === 'astro-correction'}
            <td>
              <select
                class="config-input"
                id="cfg_{name}"
                value={mcuConfig[name]}>
                <option value="0">average</option>
                <option value="1">not too late or dark</option>
                <option value="2">not too early or bright</option>
              </select>
            </td>
          {:else if name.startsWith('gpio')}
            <td>
              <McuConfigGpio {name} value={mcuConfig[name]} />
            </td>
          {:else if name !== 'gm-used'}
            <td>
              <input
                class="config-input text"
                type="text"
                id="cfg_{name}"
                {name}
                value={mcuConfig[name]} />
            </td>
          {/if}

        </tr>
      {/each}

    </table>

  </div>

  <br />
  <button id="crlb" type="button" on:click={hClick_Reload}>
    {$_('app.reload')}
  </button>
  <button id="csvb" type="button" on:click={hClick_Save}>
    {$_('app.save')}
  </button>
  <br />
  <br />
  <button id="mrtb" type="button" on:click={() => req_mcuRestart()}>
    {$_('app.restartMcu')}
  </button>
  <br />
  <div id="config_restart_div" />
  {#if $ReloadProgress > 0}
    <strong>{$_('app.msg_waitForMcuRestart')}</strong>
    <br />
    <progress id="reload_progress_bar" value={$ReloadProgress} max="100" />
  {/if}

</div>
