import React from 'react';
import pkg from 'package.json';

const depList: Record<string, string> = pkg.dependencies;

const getDepList = () => {
  depList.cbdiVersion = pkg.cbdiVersion;

  return Object.entries(depList)
    .sort((a, b) => {
      return a[0].localeCompare(b[0]);
    })
    .map((x) => {
      return (
        <div key={x[0]}>
          <span style={{ color: 'black' }}>
            {x[0]}: {x[1]}
          </span>
        </div>
      );
    });
};

function AppInfo() {
  return (
    <>
      <span style={{ color: 'black', marginLeft: '20px' }}>Dependencies: </span>
      <div style={{ margin: '20px', height: '100%', overflow: 'auto' }}>
        <ul>{getDepList()}</ul>
      </div>
    </>
  );
}

export default React.memo(AppInfo);
