/** @type {import('tailwindcss').Config} */
export default {
  content: ['./src/**/*.{js,jsx,ts,tsx}', './public/index.html'],
  theme: {
    extend: {},
    fontFamily: {
      'sans': ['BlinkMacSystemFont', 'sans-serif'],
      'serif': ['Georgia', 'Cambria', 'serif'],
    },
  },
  plugins: [],
}

