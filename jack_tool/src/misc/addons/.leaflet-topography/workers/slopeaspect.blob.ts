// @ts-nocheck

// Build a worker from an anonymous function body
export default URL.createObjectURL(
	new Blob(
		[
			'(',

			function () {
				onmessage = function (e) {
					self.slopeaspects = {};

					if (e.data.raster) {
						const {
							customization,
							RainbowAsString,
							heightFunction: heightFunctionAsString,
						} = e.data;

						const rainbowCreator = new Function('return ' + RainbowAsString);
						const Rainbow = rainbowCreator();

						const heightFunctionCreator = new Function(
							'return ' + heightFunctionAsString
						);
						const heightFunction = heightFunctionCreator();

						const { data } = e.data.raster;
						self.slopeaspects[e.data.id] = raster2slopeaspect(
							data,
							heightFunction
						);
						self.shades = shading(
							Rainbow,
							self.slopeaspects[e.data.id].slopes,
							self.slopeaspects[e.data.id].aspects,
							customization
						);
					}

					postMessage({
						id: e.data.id,
						shades: self.shades,
					});
				};

				function raster2dem(data, heightFunction) {
					const dem = new Int16Array(256 * 256);

					var x, y, i, j;

					const height =
						heightFunction ||
						function (R, G, B) {
							return -10000 + (R * 256 * 256 + G * 256 + B) * 0.1;
						};

					for (x = 0; x < 256; x++) {
						for (y = 0; y < 256; y++) {
							i = x + y * 256;
							j = i * 4;
							dem[i] = height(data[j], data[j + 1], data[j + 2]);
						}
					}

					return dem;
				}

				function raster2slopeaspect(raster) {
					const dem = raster2dem(raster);

					const aspects = new Float32Array(256 * 256);
					const slopes = new Float32Array(256 * 256);

					var x, y, dx, dy, i;

					for (x = 1; x < 255; x++) {
						for (y = 1; y < 255; y++) {
							i = y * 256 + x;

							dx =
								(dem[i - 255] +
									2 * dem[i + 1] +
									dem[i + 257] -
									(dem[i - 257] + 2 * dem[i - 1] + dem[i + 255])) /
								8;
							dy =
								(dem[i + 255] +
									2 * dem[i + 256] +
									dem[i + 257] -
									(dem[i - 257] + 2 * dem[i - 256] + dem[i - 255])) /
								8;

							aspects[i] =
								dx !== 0
									? 90 - Math.atan2(dy, -dx) * (180 / Math.PI)
									: 90 - 90 * (dy > 0 ? 1 : -1);

							slopes[i] =
								(Math.atan(Math.sqrt(dx * dx + dy * dy)) * 180) / Math.PI;
						}
					}

					/** Shameless Hack:
					 * When calculating slope, we can't get values
					 * that are on the edge of a tile, as we can't
					 * get their neighbors.  Their neighbors are on
					 * a different tile.  Rather than trying to coordinate
					 * data between tiles, this loop takes the pixel
					 * 2 pixels deep from each edge and copies it to
					 * the edge pixel.  The hack is 1px wide and barely
					 * visible
					 */
					for (x = 0; x < 256; x++) {
						for (y = 0; y < 256; y++) {
							i = y * 256 + x;

							if (x === 0) {
								j = y * 256 + x + 1;
								aspects[i] = aspects[j];
								slopes[i] = slopes[j];
							}
							if (x === 255) {
								j = y * 256 + x - 1;
								aspects[i] = aspects[j];
								slopes[i] = slopes[j];
							}
							if (y === 0) {
								j = (y + 1) * 256 + x;
								aspects[i] = aspects[j];
								slopes[i] = slopes[j];
							}
							if (y === 255) {
								j = (y - 1) * 256 + x;
								aspects[i] = aspects[j];
								slopes[i] = slopes[j];
							}
						}
					}

					return { slopes, aspects };
				}

				function shading(Rainbow, slopes, aspects, customization) {
					let continuous = false,
						userColors,
						userBreakpoints,
						fallback;

					if (customization) {
						continuous =
							customization.continuous === undefined
								? false
								: customization.continuous;
						userColors = customization.colors;
						userBreakpoints = customization.breakpoints;
						fallback = customization.fallback;
					}

					function hexToR(h) {
						return parseInt(cutHex(h).substring(0, 2), 16);
					}
					function hexToG(h) {
						return parseInt(cutHex(h).substring(2, 4), 16);
					}
					function hexToB(h) {
						return parseInt(cutHex(h).substring(4, 6), 16);
					}
					function cutHex(h) {
						return h.charAt(0) == '#' ? h.substring(1, 7) : h;
					}

					var colors = userColors || [
						'#9afb0c',
						'#00ad43',
						'#0068c0',
						'#6c00a3',
						'#ca009c',
						'#ff5568',
						'#ffab47',
						'#f4fa00',
						'#9afb0c',
					];

					const start = 0,
						end = 360,
						range = end - start,
						bracket = range / (colors.length - 1),
						offset = bracket / 2;

					const derivedBreakpoints = (() => {
						let group = [];
						group.push(start);
						for (let i = 0; i < colors.length - 1; i++) {
							let breakpoint = i * bracket + offset;
							group.push(breakpoint);
						}
						group.push(end);
						return group;
					})();

					var breakpoints = userBreakpoints || derivedBreakpoints;

					// var backup = [
					// 	0,
					// 	22.5,
					// 	67.5,
					// 	112.5,
					// 	157.5,
					// 	202.5,
					// 	247.5,
					// 	292.5,
					// 	337.5,
					// 	360,
					// ];

					var aspectGradients = (() => {
						var collection = [];

						for (let i = 0; i < breakpoints.length - 2; i++) {
							var rainbow = new Rainbow();
							rainbow.setNumberRange(breakpoints[i], breakpoints[i + 1]);
							rainbow._numberRange = [breakpoints[i], breakpoints[i + 1]];
							rainbow.setSpectrum(colors[i], colors[i + 1]);
							rainbow._spectrum = [colors[i], colors[i + 1]];
							collection.push(rainbow);
						}

						return collection;
					})();

					var gradients = colors.map((color) => {
						let rainbow = new Rainbow();
						rainbow.setNumberRange(0, 90);
						rainbow._numberRange = [0, 90];
						rainbow.setSpectrum('#808080', color);
						rainbow._spectrum = ['#808080', color];
						return rainbow;
					});

					// console.log(
					// 	'colors',
					// 	colors,
					// 	'breakpoints',
					// 	breakpoints,
					// 	'gradients',
					// 	gradients
					// );

					function hypsotint(slope, aspect) {
						let correctedAspect =
							aspect < 0
								? 360 + (aspect % 360)
								: aspect > 360
								? aspect % 360
								: aspect;

						var l = continuous ? 2 : 1;

						for (let i = 0; i < breakpoints.length - l; i++) {
							if (
								breakpoints[i] < correctedAspect &&
								correctedAspect <= breakpoints[i + 1]
							) {
								if (slope < 90) {
									if (continuous) {
										var aspectColor =
											aspectGradients[i].colorAt(correctedAspect);
										var doubleGradient = new Rainbow();
										doubleGradient.setNumberRange(0, 90);
										doubleGradient.setSpectrum('#808080', aspectColor);
										return doubleGradient.colorAt(slope);
									} else {
										return gradients[i].colorAt(slope);
									}
								}

								return colors[i];
							}
						}

						return fallback || '#00ad43';
					}

					var px = new Uint8ClampedArray(256 * 256 * 4);

					for (let i = 0; i < aspects.length; i++) {
						var hex = hypsotint(slopes[i], aspects[i]);

						px[4 * i + 0] = hexToR(hex);
						px[4 * i + 1] = hexToG(hex);
						px[4 * i + 2] = hexToB(hex);
						px[4 * i + 3] = 255;
					}

					return px;
				}
			}.toString(),

			')()',
		],
		{ type: 'application/javascript' }
	)
);
