// vite.config.js - this is to make sure the file names are not using hashes and in the same root folder
export default {
    build: {
        minify: false,
        rollupOptions: {
            output: {
                assetFileNames: "[name].[ext]",
                entryFileNames: "[name].js",
                chunkFileNames: "[name].js",
            },
        },
    },
}