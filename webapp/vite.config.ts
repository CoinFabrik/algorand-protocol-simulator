import { defineConfig } from 'vite'
import react from '@vitejs/plugin-react-swc'
// import { nodePolyfills } from 'vite-plugin-node-polyfills'

// https://vitejs.dev/config/
export default defineConfig({
  plugins: [
    react(),
    // nodePolyfills({
    //   // To add only specific polyfills, add them here. If no option is passed, adds all polyfills
    //   include: ['path', 'fs', 'buffer', 'stream', 'url', 'util', 'assert', 'os', 'constants', 'http', 'https', 'zlib', 'tty', 'process', 'sys', 'module'],
    //   // To exclude specific polyfills, add them to this list. Note: if include is provided, this has no effect
    //   exclude: [
    //      // Excludes the polyfill for `http` and `node:http`.
    //   ],
    //   // Whether to polyfill specific globals.
    //   globals: {
    //     Buffer: true, // can also be 'build', 'dev', or false
    //     global: true,
    //     process: true,
    //   },
    //   // Override the default polyfills for specific modules.
    //   overrides: {
    //     // Since `fs` is not supported in browsers, we can use the `memfs` package to polyfill it.
    //     fs: 'memfs',
    //   },
    //   // Whether to polyfill `node:` protocol imports.
    //   protocolImports: true,
    // }),
  ],
})
