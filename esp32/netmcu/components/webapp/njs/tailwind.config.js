module.exports = {
  future: {
    removeDeprecatedGapUtilities: true,
  },
  corePlugins: {
    preflight: false,
  },
  purge: [
   "./src/**/*.svelte",
   "./src/**/*.css",
   "./src/**/*.scss",
  ],
  theme: {
    extend: {
      color: {
        selected:  "#399d6e",
        bg1: "#F8F8E0",
        fg1: "#000000",


      }
    },
  },
  variants: {},
  plugins: [],
};
