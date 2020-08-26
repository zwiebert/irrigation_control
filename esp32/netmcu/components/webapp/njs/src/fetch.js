"use strict";
import * as appDebug from "./app_debug.js";
import * as httpResp from "./http_resp.js";
import { ws_isOpen } from "./net/conn_ws";
import { ZoneTimers } from "./store/zones";

let bit = 1;
export const FETCH_CONFIG = bit;
export const FETCH_ZONE_NAMES = (bit <<= 1);
export const FETCH_ZONE_LPHS = (bit <<= 1);
export const FETCH_ZONE_DURATIONS = (bit <<= 1);
export const FETCH_ZONE_REMAINING_DURATIONS = (bit <<= 1);
export const FETCH_ZONE_DATA = (bit <<= 1);
export const FETCH_ZONE_TIMERS = (bit <<= 1);
export const FETCH_RV_STATUS = (bit <<= 1);
export const FETCH_VERSION = (bit <<= 1);
export const FETCH_BOOT_COUNT = (bit <<= 1);
export const FETCH_RV_VERSION = (bit <<= 1);

export const FETCH_GIT_TAGS = 0; //XXX


const FETCHES_TARGET_STM32 = FETCH_ZONE_DATA | FETCH_ZONE_TIMERS;
const FETCHES_REPLY_BY_WS = FETCHES_TARGET_STM32 | FETCH_RV_STATUS;

const MAX_RETRY_COUNT = 3;

export function sendCmd(cmd) {
  let url = "/cmd.json";
  let obj = { from: "wapp" };
  obj.cmd = cmd;
  http_postRequest(url, obj);
}

export function sendPbuf(cmd) {
  let url = "/cmd.json";
  let obj = { from: "wapp" };
  obj.pbuf = cmd;
  http_postRequest(url, obj);
}

export function sendKvs(cmd) {
  let url = "/cmd.json";
  let obj = { from: "wapp" };
  obj.kvs = cmd;
  http_postRequest(url, obj);
}

export function sendRv(cmd) {
  let url = "/cmd.json";
  let obj = { to: "rv" };
  Object.assign(obj, cmd);
  http_postRequest(url, obj);
}

export function http_postRequest(url = "", data = {}, state = { retry_count: 0 }) {
  appDebug.dbLog("post-json: " + JSON.stringify(data));

  const fetch_data = {
    method: "POST",
    cache: "no-cache",
    headers: {
      "Content-Type": "application/json",
    },
    referrer: "no-referrer",
    body: JSON.stringify(data),
  };

  return fetch(url, fetch_data)
    .then((response) => {
      if (!response.ok) {
        console.log("error");
        if (state.retry_count++ < MAX_RETRY_COUNT) {
          return http_postRequest(url, data, state);
        }
        throw new Error("network repsonse failed");
      }
      return response.json();
    })

    .then((json) => httpResp.http_handleResponses(json))

    .catch((error) => {
      console.log("error: http_postRequest(): ", error);
    });
}

export function http_postDocRequest(name) {
  let url = "/doc";
  // Default options are marked with *
  return fetch(url, {
    method: "POST",
    cache: "no-cache",
    headers: {
      "Content-Type": "text/plain",
    },
    referrer: "no-referrer",
    body: name,
  }).then((response) => {
    if (response.ok) {
      response.text().then((text) => {
        httpResp.http_handleDocResponses(name, text);
      });
    }
  });
}

let fetchMask = 0;
function async_fetchByMask() {
  let mask = fetchMask;
  fetchMask = 0;
  if (mask & FETCHES_REPLY_BY_WS && !ws_isOpen()) {
    fetchMask = mask & FETCHES_REPLY_BY_WS;
    mask &= ~FETCHES_REPLY_BY_WS;
    setTimeout(async_fetchByMask, 125);
  }
  http_fetchByMask(mask, true);
}

export function http_fetchByMask(mask, synchron) {
  if (!mask) {
    return;
  }

  if (!synchron) {
    if (!fetchMask) setTimeout(async_fetchByMask, 125);
    fetchMask |= mask;
    return;
  }

  let mask_esp32 = mask & ~FETCHES_TARGET_STM32;
  let mask_stm32 = mask & FETCHES_TARGET_STM32;

  if (mask_esp32) fetchByMask2(mask_esp32, "netmcu");

  if (mask_stm32) fetchByMask2(mask_stm32, "rv");
}

function fetchByMask2(mask, target) {
  let tfmcu = { to: target };

  if (mask & FETCH_CONFIG) add_kv(tfmcu, "config", "all", "?");

  if (mask & FETCH_BOOT_COUNT) add_kv(tfmcu, "mcu", "boot-count", "?");

  if (mask & FETCH_VERSION) {
    add_kv(tfmcu, "mcu", "version", "?");
  }

  if (mask & FETCH_RV_VERSION) {
    add_kv(tfmcu, "cmd", "rv-version", "?");
  }

  if (mask & FETCH_ZONE_NAMES) add_kv(tfmcu, "kvs", "zn", "?");

  if (mask & FETCH_ZONE_LPHS) add_kv(tfmcu, "kvs", "lph", "?");

  if (mask & FETCH_ZONE_REMAINING_DURATIONS) {
    add_kv(tfmcu, "cmd", "rem", "?");
  }
  if (mask & FETCH_ZONE_DURATIONS) {
    add_kv(tfmcu, "cmd", "dur", "?");
  }

  if (mask & FETCH_ZONE_TIMERS) {
    ZoneTimers.set({});
    add_kv(tfmcu, "cmd", "timer", "?");
  }

  if (mask & FETCH_ZONE_DATA) add_kv(tfmcu, "pbuf", "zd", "?");

  if (mask & FETCH_RV_STATUS) {
    add_kv(tfmcu, "cmd", "status", "?");
  }

  let url = "/cmd.json";
  http_postRequest(url, tfmcu);
}

export function fetchWithTimeout(url, data, timeout) {
  return new Promise((resolve, reject) => {
    // Set timeout timer
    let timer = setTimeout(() => reject(new Error("Request timed out")), timeout);

    fetch(url, data)
      .then(
        (response) => resolve(response),
        (err) => reject(err)
      )
      .finally(() => clearTimeout(timer));
  });
}

function gitTags_fetch() {
  const tag_url = "https://api.github.com/repos/zwiebert/tronferno-mcu-bin/tags";
  const fetch_data = {
    method: "GET",
    cache: "no-cache",
    headers: {},
    referrer: "no-referrer",
    //body: JSON.stringify(data),
  };
  return fetch(tag_url, fetch_data)
    .then((response) => {
      if (!response.ok) {
        console.log("error");
        throw new Error("network repsonse failed");
      }
      return response.json();
    })

    .then((json) => httpResp.gitTags_handleResponse(json))

    .catch((error) => {
      console.log("error: httpFetch.http_postRequest(): ", error);
    });
}

function add_kv(root, cmd, key, val) {
  if (!(cmd in root)) root[cmd] = {};

  root[cmd][key] = val;
}
