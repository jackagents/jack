// © LUCAS FIELD AUTONOMOUS AGRICULTURE PTY LTD, ACN 607 923 133, 2025

class PlanInspector {
  private _inspectAgentId: string | undefined;

  private _inspectPlanGoalId: string | undefined;

  startInspectPlan(agentId: string, goalId: string) {
    this._inspectAgentId = agentId;
    this._inspectPlanGoalId = goalId;
  }

  stopInspectPlan() {
    this._inspectAgentId = undefined;
    this._inspectPlanGoalId = undefined;
  }

  isIntentionBeingInspected(agentId: string, goalId: string) {
    return agentId === this._inspectAgentId && goalId === this._inspectPlanGoalId;
  }

  getInspectingAgentId() {
    return this._inspectAgentId;
  }

  getInspectingPlanGoalId() {
    return this._inspectPlanGoalId;
  }
}

/**
 * Plan inspector singleton
 */
export const PLAN_INSPECTOR = new PlanInspector();
