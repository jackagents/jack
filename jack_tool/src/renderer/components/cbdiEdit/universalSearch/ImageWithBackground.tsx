interface ImageWithBackgroundProps {
  src: string;
  title: string;
  width: number;
  height: number;
  backgroundColor?: string;
}

export default function ImageWithBackground({
  src,
  title,
  width,
  height,
  backgroundColor,
}: ImageWithBackgroundProps) {
  return (
    <div
      style={{
        position: 'relative',
        width,
        height,
        borderRadius: '20%',
        backgroundColor: backgroundColor || 'rgba(255, 255, 255, 0.5)',
        display: 'flex',
        alignItems: 'center',
        justifyContent: 'center',
      }}
    >
      <img
        src={src}
        alt=""
        title={title}
        style={{ display: 'block', width: '70%', height: '70%' }}
      />
    </div>
  );
}
