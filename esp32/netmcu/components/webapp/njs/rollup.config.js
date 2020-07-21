// rollup.config.js
import json from '@rollup/plugin-json';
import { terser } from 'rollup-plugin-terser';
import strip from '@rollup/plugin-strip';
import { eslint } from "rollup-plugin-eslint";
import svelte from 'rollup-plugin-svelte';
import resolve from '@rollup/plugin-node-resolve';
import sveltePreprocess from 'svelte-preprocess';

export const isProduction = process.env.NODE_ENV === "production";
const useTailwind = process.env.useTailwind === "yes";

export default {
  onwarn(warning, rollupWarn) {
    if (warning.code !== 'CIRCULAR_DEPENDENCY') {
      rollupWarn(warning);
    }
  },
  input: useTailwind ? 'src/main_tailwind.js' : 'src/main.js',
  output: [...!isProduction ? [ {
    file: 'build_dev/wapp.js',
    sourcemap: true,
    format: 'iife',
    name: 'wapp',
    // sourcemapPathTransform: relativePath => {      return relativePath.substr(2);},
    plugins: [
    ]
  }] : [ {
    file: 'build/wapp.js',
    format: 'iife',
    name: 'wapp',
    sourcemap: true,
    sourcemapPathTransform: relativePath => {
      // will transform e.g. "src/main.js" -> "main.js"
      return relativePath.substr(2);
    },
    plugins: [
      terser()
    ]
  }]],

  moduleContext: (id) => {
    // In order to match native module behaviour, Rollup sets `this`
    // as `undefined` at the top level of modules. Rollup also outputs
    // a warning if a module tries to access `this` at the top level.
    // The following modules use `this` at the top level and expect it
    // to be the global `window` object, so we tell Rollup to set
    // `this = window` for these modules.
    const thisAsWindowForModules = [
        'node_modules/intl-messageformat/lib/core.js',
        'node_modules/intl-messageformat/lib/compiler.js',
        'node_modules/intl-messageformat/lib/formatters.js',
        'node_modules/intl-format-cache/lib/index.js',
        'node_modules/intl-messageformat-parser/lib/normalize.js',
        'node_modules/intl-messageformat-parser/lib/parser.js',
        'node_modules/intl-messageformat-parser/lib/skeleton.js',
    ];

    if (thisAsWindowForModules.some(id_ => id.trimRight().endsWith(id_))) {
        return 'window';
    }
  },
  plugins: [
    json(),
    ...isProduction ? [
      strip({
        functions: ['testing.*', 'testing_*', 'appDebug.*', 'console.*', 'assert.*'],
        labels: ['testing'],
        sourceMap: true,
        include: 'src/**/*.(js)',
      })] : [],
    eslint(),
    svelte({

      dev: !isProduction,
      //emitCss: true,
      css: css => { css.write(isProduction ? 'build/wapp.css' : 'build_dev/wapp.css'); },
      onwarn: (warning, handler) => {
        // e.g. don't warn on <marquee> elements, cos they're cool
        if (warning.code === 'a11y-no-onchange') return;

        // let Rollup handle all other warnings normally
        handler(warning);
      },
      preprocess: [
        sveltePreprocess({
        postcss: true,
        replace: [
            ['misc.NODE_ENV_DEV', isProduction ? 'false' : 'true'],
            ['//NODE_ENV_DEV', isProduction ? 'if(false)' : 'if(true)'],
         ],
      })],

    }),
    // If you have external dependencies installed from
    // npm, you'll most likely need these plugins. In
    // some cases you'll need additional configuration -
    // consult the documentation for details:
    // https://github.com/rollup/plugins/tree/master/packages/commonjs
    resolve({
      browser: true,
      dedupe: ['svelte']
    })
  ]
};

