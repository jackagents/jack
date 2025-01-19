import React from 'react';

const EmptyDetail: React.FC = () => {
  return (
    <div style={{display:"flex", justifyContent:'center',alignItems:"center",height:'100%'}}>
      <h2 style={{color:'gray'}}>No Node Selected</h2>
    </div>
  );
};

export default EmptyDetail;