export const editorRendererToRendererEvents = {
  editorAddingPlanTask: 'editor-adding-plan-task',
  editorAddingPlanTaskWithEdge: 'editor-adding-plan-task-with-edge',
  editorExportPlanPngGraph: 'editor-export-plan-png-graph',
  editorOpenUniversalSearchWithPlanEdge: 'editor-open-universal-search-with-plan-edge',
  /**
   * when need to add nodeIds to project tree expanded
   */
  editorAddToProjectTreeExpanded: 'editor-add-to-project-tree-expanded',
};

export const request = {
  window: {
    minimize: 'window-minimize',
    maximize: 'window-maximize',
    close: 'window-close',
  },
  theme: {
    setTheme: 'request-cbdiEdit-set-theme',
    getTheme: 'request-cbdiEdit-get-theme',
  },
  project: {
    new: 'request-cbdiEdit-project-new',
    open: 'request-cbdiEdit-project-open',
    openWithPath: 'request-cbdiEdit-project-open-with-path',
    update: 'request-cbdiEdit-project-update',
    saveAs: 'request-cbdiEdit-project-saveAs',
    save: 'request-cbdiEdit-project-save',
    overwrite: 'request-cbdiEdit-project-overwrite',
    clearFileWatcher: 'request-cbdiEdit-project-clear-file-watcher',
    close: 'request-cbdiEdit-project-close',
    createModule: 'request-cbdiEdit-project-module-create',
    importModule: 'request-cbdiEdit-project-module-import',
    removeModule: 'request-cbdiEdit-project-module-remove',
    renameModule: 'request-cbdiEdit-project-module-rename',
  },
  graph: {
    compute: 'request-compute-graph',
    generatePlan: 'request-generate-plan-graph',
  },
};

export const response = {
  window: {
    maximized: 'window-maximized',
  },
  theme: {
    themeChanged: 'response-cbdiEdit-theme-changed',
  },
  project: {
    save: 'response-cbdiEdit-project-save',
    saveAs: 'response-cbdiEdit-project-saveAs',
    changed: 'response-cbdiEdit-project-changed',
    exitSaving: 'response-cbdiEdit-project-exitSaving',
    moduleChanged: 'response-cbdiEdit-project-module-changed',
    saveSuccess: 'response-cbdiEdit-project-save-success',
  },
  graph: {
    compute: 'response-compute-graph',
    generatePlan: 'response-generate-plan-graph',
  },
};
