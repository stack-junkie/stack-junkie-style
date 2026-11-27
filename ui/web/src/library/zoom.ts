// Zoom steps shared by the App frame's zoom commands, web and native: Zoom in and Zoom out move one step, Reset zoom returns to 100.
export const zoomSteps = [50, 67, 75, 80, 90, 100, 110, 125, 150, 175, 200] as const;
export const defaultZoom = 100;
/** The next zoom step up (direction 1) or down (-1) from any percentage, staying within the steps. */
export function stepZoom(current: number, direction: 1 | -1): number {
  if (direction > 0) return zoomSteps.find(step => step > current) ?? zoomSteps[zoomSteps.length - 1];
  return [...zoomSteps].reverse().find(step => step < current) ?? zoomSteps[0];
}
/** A stored zoom percentage, or 100 when it is missing or outside the steps' range. */
export function readZoom(value: unknown): number {
  const number = Number(value);
  return Number.isFinite(number) && number >= zoomSteps[0] && number <= zoomSteps[zoomSteps.length - 1] ? number : defaultZoom;
}
