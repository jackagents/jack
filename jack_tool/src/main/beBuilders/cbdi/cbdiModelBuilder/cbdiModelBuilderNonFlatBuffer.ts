// © LUCAS FIELD AUTONOMOUS AGRICULTURE PTY LTD, ACN 607 923 133, 2025

import { CBDIAgent, CBDIService, CBDITeam } from 'misc/types/cbdi/cbdi-models-non-flatbuffer';
import { BaseBuilder } from 'main/beBuilders/baseBuilder/BaseBuilder';
import {
  AgentJoinTeamBody,
  DelegationBody,
  Event,
  EventType,
  MessageBody,
  NodeType,
  PerceptBody,
  PursueBody,
  RegisterBody,
} from 'misc/types/cbdi/cbdi-types-non-flatbuffer';
import LOGGER from 'misc/addons/logger/LoggerSingleton';
import { ipcMain } from 'electron';
import { eventListeners } from 'projectEvents/common/cmEvents';

export class CBDIModelBuilder extends BaseBuilder {
  private _nodeInfo: CBDIAgent | null = null;

  private _agents: CBDIAgent[] = [];

  private _services: CBDIService[] = [];

  private _proxyAgents: CBDIAgent[];

  private _proxyServices: CBDIService[];

  private _originalAgent: CBDIAgent | null = null;

  private _originalModel: Record<string, CBDIAgent | CBDIService> | null = null;

  private delegationFlag = false;

  /* -------------------------------------------------------------------------- */
  /*                                 CONSTRUCTOR                                */
  /* -------------------------------------------------------------------------- */

  constructor(playback = false) {
    super('CBDIModelBuilder', playback);

    this._proxyAgents = new Proxy(this._agents, this.agentsProxyHandler());

    this._proxyServices = new Proxy(this._services, this.servicesProxyHandler());
  }

  private agentsProxyHandler() {
    // eslint-disable-next-line @typescript-eslint/no-this-alias
    const context = this;

    return {
      get(target: any, p: string | symbol) {
        if (p === 'push') {
          return (...args: any) => {
            target[p](...args);

            if (!context._playback) {
              ipcMain.emit(eventListeners.cbdi.modelUpdated);
            }
            return true;
          };
        }

        return target[p];
      },
      set(target: any, p: string | symbol, newValue: any) {
        target[p] = newValue;
        return true;
      },
    };
  }

  private servicesProxyHandler() {
    // eslint-disable-next-line @typescript-eslint/no-this-alias
    const context = this;

    return {
      get(target: any, p: string | symbol) {
        if (p === 'push') {
          return (...args: any) => {
            target[p](...args);

            if (!context._playback) {
              ipcMain.emit(eventListeners.cbdi.modelUpdated);
            }
            return true;
          };
        }

        return target[p];
      },
      set(target: any, p: string | symbol, newValue: any) {
        target[p] = newValue;
        return true;
      },
    };
  }

  private agentValidator() {
    // eslint-disable-next-line @typescript-eslint/no-this-alias
    const context = this;
    return {
      get(target: CBDIAgent, key: string): any {
        if (typeof target[key] === 'object' && target[key] !== null) {
          return new Proxy(target[key], context.agentValidator());
        }

        return target[key];
      },
      set(target: CBDIAgent, key: string, value: any) {
        target[key] = value;

        if (!context._playback && (key === 'teams' || key === 'members')) {
          ipcMain.emit(eventListeners.cbdi.modelUpdated);
        }

        return true;
      },
    };
  }

  private serviceValidator() {
    // eslint-disable-next-line @typescript-eslint/no-this-alias
    const context = this;
    return {
      get(target: CBDIService, key: string): any {
        if (typeof target[key] === 'object' && target[key] !== null) {
          return new Proxy(target[key], context.serviceValidator());
        }

        return target[key];
      },
      set(target: CBDIService, key: string, value: any) {
        target[key] = value;

        if (!context._playback) {
          ipcMain.emit(eventListeners.cbdi.modelUpdated);
        }

        return true;
      },
    };
  }

  /* -------------------------------------------------------------------------- */
  /*                                   PRIVATE                                  */
  /* -------------------------------------------------------------------------- */

  private _searchAgent = (id: string) => this._proxyAgents.find((x) => x.address.id === id);

  private _searchService = (id: string) => this._proxyServices.find((x) => x.address.id === id);

  private _createProxyAgent = (target: CBDIAgent) => new Proxy(target, this.agentValidator());

  private _createProxyService = (target: CBDIService) => new Proxy(target, this.serviceValidator());

  /* -------------------------------------------------------------------------- */
  /*                                   PUBLIC                                   */
  /* -------------------------------------------------------------------------- */

  clear() {
    this._proxyAgents.length = 0;
    this._proxyServices.length = 0;
  }

  reset() {
    this._nodeInfo = null;
    this._agents.length = 0;
    this._services.length = 0;
    this._proxyAgents.length = 0;
    this._proxyServices.length = 0;
  }

  /**
   * Create snapshot of model builder
   * @returns
   */
  createSnapshot() {
    return JSON.stringify({
      nodeInfo: this._nodeInfo,
      agents: this._agents,
      services: this._services,
      proxyAgents: this._proxyAgents,
      proxyServices: this._proxyServices,
      originalAgent: this._originalAgent,
      originalModel: this._originalModel,
      delegationFlag: this.delegationFlag,
    });
  }

  /**
   * Restore snapshot to model builder
   * @param snapshotString
   */
  restoreSnapshot(snapshotString: string) {
    this.clear();
    const { agents, delegationFlag, nodeInfo, originalAgent, originalModel, services } = JSON.parse(snapshotString) as {
      nodeInfo: CBDIAgent | null;
      agents: CBDIAgent[];
      services: CBDIService[];
      proxyAgents: CBDIAgent[];
      proxyServices: CBDIService[];
      originalAgent: CBDIAgent | null;
      originalModel: Record<string, CBDIAgent | CBDIService> | null;
      delegationFlag: boolean;
    };

    this._nodeInfo = nodeInfo;
    this._originalAgent = originalAgent;
    this._originalModel = originalModel;
    this._agents = agents.map((a) => this._createProxyAgent(a));
    this._services = services.map((s) => this._createProxyService(s));
    this._proxyAgents = new Proxy(this._agents, this.agentsProxyHandler());
    this._proxyServices = new Proxy(this._services, this.servicesProxyHandler());
    this.delegationFlag = delegationFlag;
  }

  /**
   * Build agents model
   * @param param0
   * @returns void
   */
  buildNonFlatBuffer(data: Event) {
    const { type, sender, recipient, eventId, senderNode, timestampUs, ...rest } = data;

    switch (type) {
      case EventType.REGISTER: {
        const body = rest as RegisterBody;

        // LOGGER.debug('Register', JSON.stringify(body));
        // Node
        if (body.address.type === NodeType.NODE) {
          if (!this._nodeInfo) {
            // Emit to front-end on first heartbeat
            this._nodeInfo = {
              ...body,
              lastUpdated: data.timestampUs,
            } as CBDIAgent;

            ipcMain.emit(eventListeners.cbdi.nodeInfo);
            return;
          }

          this._nodeInfo.lastUpdated = data.timestampUs;
          return;
        }

        // Service
        if (body.address.type === NodeType.SERVICE) {
          let service = this._searchService(body.address.id);

          if (service) {
            service = { ...body, lastUpdated: data.timestampUs };
            return;
          }

          const newService = this._createProxyService({
            ...body,
            lastUpdated: data.timestampUs,
          });

          this._proxyServices.push(newService);
          return;
        }

        if (body.address.type === NodeType.AGENT || body.address.type === NodeType.TEAM) {
          const agent = this._searchAgent(body.address.id);

          // Update agent info
          if (agent) {
            agent.templateType = body.templateType;
            agent.address = body.address;
            agent.proxy = body.proxy;
            agent.team = body.team;
            agent.start = body.start;
            agent.lastUpdated = data.timestampUs;
            return;
          }

          // Create new agent
          const newAgent = {
            ...body,
            teams: body.team ? [body.team] : [],
            lastUpdated: data.timestampUs,
          } as CBDIAgent;

          const mProxiedAgent = this._createProxyAgent(newAgent);

          this._proxyAgents.push(mProxiedAgent);
        }

        break;
      }

      case EventType.AGENT_JOIN_TEAM: {
        const body = rest as AgentJoinTeamBody;

        // Find team
        let team: CBDITeam | undefined = this._searchAgent(body.team.id);

        // Find agent
        let agent = this._searchAgent(body.agent.id);

        // LOGGER.debug('Agent join team', JSON.stringify(body));

        if (!agent) {
          LOGGER.warn('Agent join team failed, cannot find agent');

          // Create proxy agent
          agent = this._createProxyAgent({
            proxy: false,
            address: body.agent,
            templateType: 'unknown',
            start: true,
            teams: [],
            lastUpdated: data.timestampUs,
          } as CBDIAgent);

          this._proxyAgents.push(agent);
        }

        if (!team) {
          LOGGER.warn('Agent join team failed, cannot find team');

          // Create proxy team
          team = this._createProxyAgent({
            proxy: false,
            address: body.team,
            templateType: 'unknown',
            start: true,
            teams: [],
            members: [],
            lastUpdated: data.timestampUs,
          } as CBDITeam);

          this._proxyAgents.push(team);
        }

        if (agent.teams?.find((x) => x.id === team?.address.id)) {
          LOGGER.warn(`Agent ${agent.address.name} already in the team ${team.address.name}`);
          return;
        }

        // Update agent's teams
        if (agent.teams && agent.teams.length > 0) {
          agent.teams = [...agent.teams, team.address];
        } else {
          agent.teams = [team.address];
        }

        // Init team members
        if (!team.members) {
          team.members = [];
        }

        // Add member to team
        if (team.members.map((x) => x.id).indexOf(agent.address.id) < 0) {
          team.members.push(agent.address);
        }

        // Update timestamp
        agent.lastUpdated = data.timestampUs;
        team.lastUpdated = data.timestampUs;

        break;
      }

      case EventType.PURSUE: {
        const body = rest as PursueBody;

        // Find agent
        let agent = this._searchAgent(recipient.id);

        // LOGGER.debug('Pursue', JSON.stringify(data));

        if (!agent) {
          LOGGER.warn('Cannot find recipient');

          agent = {
            proxy: false,
            address: recipient,
            templateType: 'unknown',
            start: true,
            teams: [],
            lastUpdated: timestampUs,
          } as CBDIAgent;

          const mProxiedAgent = this._createProxyAgent(agent);
          this._proxyAgents.push(mProxiedAgent);
        }
        if (agent.pursues === undefined) {
          agent.pursues = {};
        }

        if (agent.pursues[eventId] === undefined) {
          agent.pursues[eventId] = [];
        }

        agent.pursues[eventId].push(body);
        agent.lastUpdated = timestampUs;

        break;
      }
      // TODO: Implement message handling
      // case EventType.MESSAGE: {
      //   const body = rest as MessageBody;

      //   // Find agent
      //   const agent = this._searchAgent(recipient.id);

      //   if (!agent) {
      //     if (recipient.id !== '') {
      //       LOGGER.info('Cannot find recipient', recipient.id);
      //       const newAgent = {
      //         address: recipient,
      //         lastUpdated: data.timestampUs,
      //       } as CBDIAgent;

      //       const mProxiedAgent = this._createProxyAgent(newAgent);
      //       this._proxyAgents.push(this._createProxyAgent(mProxiedAgent));
      //     }
      //     return;
      //   }

      //   agent.lastUpdated = timestampUs;

      //   // Init beliefsets
      //   if (!agent.beliefSets) {
      //     agent.beliefSets = {};
      //   }

      //   // Get beliefset key
      //   const beliefSetKey = body.schema;

      //   // Update beliefs
      //   agent.beliefSets[beliefSetKey] = {
      //     ...agent.beliefSets[beliefSetKey],
      //     ...body.fields,
      //   };

      //   break;
      // }

      case EventType.DELEGATION: {
        const body = rest as DelegationBody;

        // Find agent
        let agent = this._searchAgent(recipient.id);

        // LOGGER.info('Delegation', JSON.stringify(body.delegation));
        if (!agent) {
          LOGGER.warn('Cannot find recipient');

          agent = {
            proxy: false,
            address: recipient,
            templateType: 'unknown',
            start: true,
            teams: [],
            lastUpdated: timestampUs,
          } as CBDIAgent;

          const mProxiedAgent = this._createProxyAgent(agent);
          this._proxyAgents.push(mProxiedAgent);
        }

        agent.lastUpdated = timestampUs;

        // If analyse = true, store to auctions
        if (body.analyse) {
          if (!agent.auctions) {
            agent.auctions = [];
          }

          const auctionIndex = agent.auctions.findIndex((x) => x.goalId === body.goalId);

          // update auction
          if (auctionIndex > -1) {
            agent.auctions[auctionIndex] = body;
          }
          // add new
          else {
            agent.auctions.push(body);
          }
        }

        // If analyse = false, store to currentDelegation
        if (!body.analyse) {
          if (!agent.delegations) {
            agent.delegations = [];
          }

          const delegationIndex = agent.delegations.findIndex((x) => x.goalId === body.goalId);

          // update auction
          if (delegationIndex > -1) {
            agent.delegations[delegationIndex] = body;
          }
          // add new
          else {
            agent.delegations.push(body);
          }
        }

        // Only poke client every x seconds
        if (!this.delegationFlag) {
          this.delegationFlag = true;
          setTimeout(() => {
            this.delegationFlag = false;
            ipcMain.emit(eventListeners.cbdi.modelUpdated);
          }, 3000);
        }
        break;
      }

      case EventType.PERCEPT: {
        const body = rest as PerceptBody;

        // Find agent
        let agent = this._searchAgent(recipient.id);

        if (!agent) {
          LOGGER.warn('Cannot find recipient');

          agent = {
            proxy: false,
            address: data.recipient,
            templateType: 'unknown',
            start: true,
            teams: [],
            lastUpdated: data.timestampUs,
          } as CBDIAgent;

          const mProxiedAgent = this._createProxyAgent(agent);
          this._proxyAgents.push(mProxiedAgent);
        }

        agent.lastUpdated = timestampUs;

        // Init beliefsets
        if (!agent.beliefSets) {
          agent.beliefSets = {};
        }
        const { beliefSets } = agent;

        // Init beliefs
        if (!beliefSets[body.beliefSet]) {
          beliefSets[body.beliefSet] = {};
        }

        if (body.field) {
          const { field } = body;
          beliefSets[body.beliefSet] = {
            ...beliefSets[body.beliefSet],
            ...field,
          };
        }

        break;
      }

      case EventType.BDI_LOG: {
        // Is being processed in intention builder and logs builder
        break;
      }

      case EventType.ACTION_BEGIN: {
        // Ignore atm
        // const body = rest as ActionBeginBody;
        break;
      }

      case EventType.ACTION_UPDATE: {
        // Ignore atm
        // const body = rest as ActionUpdateBody;
        break;
      }

      case EventType.CONTROL: {
        // Ignore atm
        // const body = rest as ControlBody;
        break;
      }

      case EventType.DROP: {
        // Ignore atm
        // const body = rest as DropBody;
        break;
      }

      case EventType.DEREGISTER: {
        // Ignore atm
        // const body = rest as DeregisterBody;
        // const { id, nodeType } = body;

        break;
      }

      case EventType.COUNT: {
        throw new Error('COUNT is not implemented');
      }

      case EventType.NONE: {
        throw new Error('NONE is not implemented');
      }

      case EventType.AGENT_LEAVE_TEAM: {
        throw new Error('AGENT_LEAVE_TEAM is not implemented');
      }

      default:
        break;
    }
  }

  /**
   * Get all agents
   * @returns CBDIAgent[]
   */
  getAgents() {
    return this._proxyAgents;
  }

  getServices() {
    return this._proxyServices;
  }

  getModels() {
    const result: Record<string, CBDIAgent | CBDIService> = {};
    const data = [...this._proxyAgents, ...this._proxyServices];
    const keys = data.map((x) => x.address.id);

    keys.forEach((key, index) => {
      result[key] = data[index];
    });

    return result;
    // return [...this._proxyAgents, ...this._proxyServices];
  }

  /**
   * Get agent by id
   * @param id string
   * @returns CBDIAgent
   */
  getAgent(id: string) {
    return this._searchAgent(id);
  }

  requestAgentChanges(id: string, grabFresh: boolean) {
    if (grabFresh) {
      this._originalAgent = null;
    }

    const memoryAgent = this._searchAgent(id);

    if (memoryAgent) {
      if (this._originalAgent) {
        const changes = this.calculateDifferences(this._originalAgent, memoryAgent);

        this._originalAgent = JSON.parse(JSON.stringify(memoryAgent));
        return changes;
      }

      this._originalAgent = JSON.parse(JSON.stringify(memoryAgent));
      return this._originalAgent;
    }

    return null;
  }

  requestModelChanges(grabFresh: boolean) {
    if (grabFresh) {
      this._originalModel = null;
    }

    const memoryModel = this.getModels();

    if (this._originalModel) {
      const changes = this.calculateDifferences(this._originalModel, memoryModel);

      this._originalModel = JSON.parse(JSON.stringify(memoryModel));
      return changes;
    }

    this._originalModel = JSON.parse(JSON.stringify(memoryModel));
    return this._originalModel;
  }

  getNodeInfo() {
    return this._nodeInfo;
  }

  /**
   * Get goal context using goal id and agent id
   * @param agentId
   * @param goalId
   * @returns
   */
  getPursueGoalContext(agentId: string, goalId: string): PursueBody[] | undefined {
    const agent = this._searchAgent(agentId);
    return agent && agent.pursues ? agent.pursues[goalId] : undefined;
  }
}

const MODEL_BUILDER = new CBDIModelBuilder();
export default MODEL_BUILDER;
