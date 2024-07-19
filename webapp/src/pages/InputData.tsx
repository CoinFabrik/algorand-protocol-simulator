import { useContext, useState } from 'react'
import { Link } from 'react-router-dom'
import Context from '../context/Context'

function InputData (): JSX.Element {
  // Get input data
  const [data, setData] = useState({
    relayNodes: 0,
    participationNodes: 0,
    relayConnectionDelay: 0,
    connectionDensity: 0,
    numberOfAccounts: 0,
    totalSupplyOfAlgos: 0,
    balanceInAlgos: 0,
    accountsPerNode: 0,
    onlineAccounts: 0,
    commiteesValues: 0,
    seedLookback: 0,
    seedRefreshInterval: 0,
    balanceLookback: 0,
    lambda0: 0,
    lambdaf: 0,
    lambda: 0,
    lambdam: 0,
    finalizingCondition: 0,
    transactionPoolLimit: 0,
    ledgerCache: 0,
    simulateVRF: 0
  })

  const { setContext } = useContext(Context)

  // Handle input data
  const handleData = (e: React.ChangeEvent<HTMLInputElement>): void => {
    setData({
      ...data,
      [e.target.name]: e.target.value
    })
    // console.log(data)
    console.log(data)
  }

  // Actualizar el estado cuando hace click

  const handleSimulate = (): void => {
    // address de cuenta - balance - status(online/ofline) - nodo

    // 100 relays
    // 1000 participacion
    // si es 50% de densidad de conexion
    // cada nodo de participacion se conecta a 50 relays random

    // cantidad de nodos - cantidad de nodos de relay
    // 2000 120

    // cada nodo de participacion se puede conectar a los nodos relay que quiera
    // nodo 0 (participacion-relay): 27 0 0 52 0 0 14 0 0 54 0 0

    // cada nodo de participacion se conecta a 4 relays

    // primer numero: id del relay
    // segundo numero: delay de conexion de entrada
    // tercer numero: delay de conexion de salida
    // cuarto numero: id del nodo de relay de conexion
    // quinto numero: delay de conexion de entrada
    // sexto numero: delay de conexion de salida
    // septimo numero: id del nodo de relay de conexion

    // cuando conecto todos los nodos de participacion a los nodos de relay
    // defino las conexiones de un relay a relay
    // el relay 0 se conecta: 5 0  0  9 0  0  23 0  0  27 0  0  29 0  0  32 0  0  33 0  0  37 0  0  43 0  0  54 0  0  57 0  0  68 0  0  69 0  0  73 0  0  80 0  0  87 0  0  89 0  0  96 0  0
    // (10-20 relays)
    // id del nodo de relay
    // no se conectan asi mismos ni repetidos

    // Create network file

    // const networkSimulator = [[Number(data.participationNodes), Number(data.relayNodes)]]

    const networkInfo = {
      nodes: Array<Record<string, unknown>>(),
      links: Array<Record<string, unknown>>()
    }

    // LINKS

    const totalPossibleConnectionsRelay = (Number(data.relayNodes) * (Number(data.relayNodes) - 1))

    const quantityOfConnectionsRelay = Math.round(totalPossibleConnectionsRelay * (Number(data.connectionDensity) / 100))

    const orderedConnections: Array<Record<string, unknown>> = [{}]

    for (let i = 0; i < Number(data.relayNodes); i++) {
      for (let j = i + 1; j < Number(data.relayNodes); j++) {
        orderedConnections.push({
          source: i,
          target: j
        })
      }
    }

    const shuffledConnections = orderedConnections
      .map(value => ({ value, sort: Math.random() }))
      .sort((a, b) => a.sort - b.sort)
      .map(({ value }) => value)

    for (let i = 0; i < quantityOfConnectionsRelay; i++) {
      networkInfo.links.push({
        source: shuffledConnections[i].source,
        target: shuffledConnections[i].target,
        value: 1
      })
    }

    const totalPossibleConnectionsParticipation = Number(data.participationNodes) * Number(data.relayNodes)

    const quantityOfConnectionsParticipation = Math.round(totalPossibleConnectionsParticipation * (Number(data.connectionDensity) / 100))

    const orderedParticipation: Array<Record<string, unknown>> = [{}]

    for (let i = Number(data.relayNodes); i < Number(data.relayNodes) + Number(data.participationNodes); i++) {
      for (let j = 0; j < data.relayNodes; j++) {
        orderedParticipation.push({
          source: i,
          target: j
        })
      }
    }

    const shuffledConnectionsParticipation = orderedParticipation
      .map(value => ({ value, sort: Math.random() }))
      .sort((a, b) => a.sort - b.sort)
      .map(({ value }) => value)

    for (let i = 0; i < quantityOfConnectionsParticipation; i++) {
      networkInfo.links.push({
        source: shuffledConnectionsParticipation[i].source,
        target: shuffledConnectionsParticipation[i].target,
        value: 1
      })
    }

    // NODES
    for (let i = 0; i < Number(data.relayNodes); i++) {
      networkInfo.nodes.push({
        id: i,
        type: 'relay'
      })
    }

    for (let i = Number(data.relayNodes); i < Number(data.relayNodes) + Number(data.participationNodes); i++) {
      networkInfo.nodes.push({
        id: i,
        type: 'participation'
      })
    }

    const newNetworkInfo = JSON.stringify(networkInfo)
    const dataBlobNetwork = new Blob([newNetworkInfo], { type: 'application/json' })
    const blobURLNetwork = URL.createObjectURL(dataBlobNetwork)
    const downloadLinkNetwork = document.createElement('a')
    downloadLinkNetwork.href = blobURLNetwork
    downloadLinkNetwork.download = 'test_network.json'
    downloadLinkNetwork.click()
    URL.revokeObjectURL(blobURLNetwork)

    console.log(networkInfo)

    setContext(networkInfo)

    // const newNetworkSimulator = JSON.stringify(networkSimulator)
    // const dataBlobNetworkSimulator = new Blob([newNetworkSimulator], { type: 'text/plain' });
    // const blobURLNetworkSimulator = URL.createObjectURL(dataBlobNetworkSimulator);
    // const downloadLinkNetworkSimulator = document.createElement('a')
    // downloadLinkNetworkSimulator.href = blobURLNetworkSimulator;
    // downloadLinkNetworkSimulator.download = "test_network_simulator.txt";
    // downloadLinkNetworkSimulator.click();
    // URL.revokeObjectURL(blobURLNetworkSimulator);
  }

  return (
    <div>
        <h1 className="text-center font-bold text-4xl mt-5">Algorand Blockchain Simulator</h1>
        <p className="text-center text-lg mb-14">Before start is required the parameters of simulation</p>
        <div className="flex flex-row justify-center">
            <div className="w-1/4">
                <h2 className="text-center font-bold text-4xl">Network</h2>
                <h3 className="text-center text-medium mt-5 mb-1">Relay nodes</h3>
                <input
                    className="ml-16 text-center pl-2 p-1 w-2/3 border-2 border-slate-300 rounded-xl"
                    type='number'
                    placeholder='7893'
                    onChange={handleData}
                    name='relayNodes'
                />
                <h3 className="text-center text-medium mt-5 mb-1">Participation nodes</h3>
                <input
                    className="ml-16 text-center pl-2 p-1 w-2/3 border-2 border-slate-300 rounded-xl"
                    type='number'
                    placeholder='1456'
                    onChange={handleData}
                    name='participationNodes'
                />
                <h3 className="text-center text-medium mt-5 mb-1">Relay connection Delay (ms)</h3>
                <input
                    className="ml-16 text-center pl-2 p-1 w-2/3 border-2 border-slate-300 rounded-xl"
                    type='number'
                    placeholder='20'
                    onChange={handleData}
                    name='relayConnectionDelay'
                />
                <h3 className="text-center text-medium mt-5 mb-1">Conection Density (%)</h3>
                <input
                    className="ml-16 text-center pl-2 p-1 w-2/3 border-2 border-slate-300 rounded-xl"
                    type='number'
                    placeholder='48'
                    onChange={handleData}
                    name='connectionDensity'
                />
            </div>
            <div className="w-1/4">
                <h2 className="text-center font-bold text-4xl">Accounts & Balances</h2>
                <h3 className="text-center text-medium mt-5 mb-1">Number of Accounts</h3>
                <input
                    className="ml-16 text-center pl-2 p-1 w-2/3 border-2 border-slate-300 rounded-xl"
                    type='number'
                    placeholder='159734'
                    onChange={handleData}
                    name='numberOfAccounts'
                />
                <h3 className="text-center text-medium mt-5 mb-1">Total supply of Algos</h3>
                <input
                    className="ml-16 text-center pl-2 p-1 w-2/3 border-2 border-slate-300 rounded-xl"
                    type='number'
                    placeholder='201201201'
                    onChange={handleData}
                    name='totalSupplyOfAlgos'
                />
                <h3 className="text-center text-medium mt-5 mb-1">Balance in Algos</h3>
                <input
                    className="ml-16 text-center pl-2 p-1 w-2/3 border-2 border-slate-300 rounded-xl"
                    type='number'
                    placeholder='1456'
                    onChange={handleData}
                    name='balanceInAlgos'
                />
                <h3 className="text-center text-medium mt-5 mb-1">Accounts per node</h3>
                <input
                    className="ml-16 text-center pl-2 p-1 w-2/3 border-2 border-slate-300 rounded-xl"
                    type='number'
                    placeholder='10'
                    onChange={handleData}
                    name='accountsPerNode'
                />
                <h3 className="text-center text-medium mt-5 mb-1">Online Accounts (%)</h3>
                <input
                    className="ml-16 text-center pl-2 p-1 w-2/3 border-2 border-slate-300 rounded-xl"
                    type='number'
                    placeholder='23'
                    onChange={handleData}
                    name='onlineAccounts'
                />
            </div>
            <div className="w-1/4">
                <h2 className="text-center font-bold text-4xl">Consensus</h2>
                <h3 className="text-center text-medium mt-5 mb-1">Commites Values</h3>
                <input
                    className="ml-16 text-center pl-2 p-1 w-2/3 border-2 border-slate-300 rounded-xl"
                    type='number'
                    placeholder='7893'
                    onChange={handleData}
                    name='commiteesValues'
                />
                <h3 className="text-center text-medium mt-5 mb-1">Seed Lookback</h3>
                <input
                    className="ml-16 text-center pl-2 p-1 w-2/3 border-2 border-slate-300 rounded-xl"
                    type='number'
                    placeholder='1456'
                    onChange={handleData}
                    name='seedLookback'
                />
                <h3 className="text-center text-medium mt-5 mb-1">Seed refresh interval (ms)</h3>
                <input
                    className="ml-16 text-center pl-2 p-1 w-2/3 border-2 border-slate-300 rounded-xl"
                    type='number'
                    placeholder='20'
                    onChange={handleData}
                    name='seedRefreshInterval'
                />
                <h3 className="text-center text-medium mt-5 mb-1">Balance Lookback</h3>
                <input
                    className="ml-16 text-center pl-2 p-1 w-2/3 border-2 border-slate-300 rounded-xl"
                    type='number'
                    placeholder='48'
                    onChange={handleData}
                    name='balanceLookback'
                />
                <div className="flex flex-row ml-16 justify-center">
                    <div className="w-1/3">
                        <h3 className="text-medium mt-5 mb-1">λ0 (ms)</h3>
                        <input
                            className="text-center pl-2 p-1 w-2/3 border-2 border-slate-300 rounded-xl"
                            type='number'
                            placeholder='6'
                            onChange={handleData}
                            name='lambda0'
                        />
                    </div>
                    <div className="w-1/3">
                        <h3 className="text-medium mt-5 mb-1">λf (ms)</h3>
                        <input
                            className="text-center pl-2 p-1 w-2/3 border-2 border-slate-300 rounded-xl"
                            type='number'
                            placeholder='23'
                            onChange={handleData}
                            name='lambdaf'
                        />
                    </div>
                </div>
                <div className="flex flex-row ml-16 justify-center">
                <div className="w-1/3">
                        <h3 className="text-medium mt-5 mb-1">λ (ms)</h3>
                        <input
                            className="text-center pl-2 p-1 w-2/3 border-2 border-slate-300 rounded-xl"
                            type='number'
                            placeholder='45'
                            onChange={handleData}
                            name='lambda'
                        />
                    </div>
                    <div className="w-1/3">
                        <h3 className="text-medium mt-5 mb-1">Λ (ms)</h3>
                        <input
                            className="text-center pl-2 p-1 w-2/3 border-2 border-slate-300 rounded-xl"
                            type='number'
                            placeholder='4'
                            onChange={handleData}
                            name='lambdam'
                        />
                    </div>
                </div>
            </div>
            <div className="w-1/4">
                <h2 className="text-center font-bold text-4xl">Simulation</h2>
                <h3 className="text-center text-medium mt-5 mb-1">Finalizing condition</h3>
                <input
                    className="ml-16 text-center pl-2 p-1 w-2/3 border-2 border-slate-300 rounded-xl"
                    type='select'
                    placeholder='7893'
                    onChange={handleData}
                    name='finalizingCondition'
                />
                <h3 className="text-center text-medium mt-5 mb-1">Transaction pool limit</h3>
                <input
                    className="ml-16 text-center pl-2 p-1 w-2/3 border-2 border-slate-300 rounded-xl"
                    type='number'
                    placeholder='1456'
                    onChange={handleData}
                    name='transactionPoolLimit'
                />
                <h3 className="text-center text-medium mt-5 mb-1">Ledger Caché</h3>
                <input
                    className="ml-16 text-center pl-2 p-1 w-2/3 border-2 border-slate-300 rounded-xl"
                    type='number'
                    placeholder='20'
                    onChange={handleData}
                    name='ledgerCache'
                />
                <h3 className="text-center text-medium mt-5 mb-1">Simulate VRF</h3>
                <input
                    className="ml-16 text-center pl-2 p-1 w-2/3 border-2 border-slate-300 rounded-xl"
                    type='bool'
                    placeholder='48'
                    onChange={handleData}
                    name='simulateVRF'
                />
            </div>
        </div>
        {/* <div className="text-right mr-16 text-white">
            <Link onClick={handleSimulate} to='/Graph' id='simulate' className="bg-blue-500 rounded-xl p-3">
                Simulate &rarr;
            </Link>
        </div> */}
        {/** Add upload data json */}
        <div className="text-right mr-16 text-white">
            <input type="file"
              className="text-md text-white file:mr-1 file:py-1 file:px-3 file:border-[0px] file:rounded-xl file:text-md file:font-medium file:bg-stone-50 file:text-stone-700 hover:file:cursor-pointer hover:file:bg-blue-50 hover:file:text-blue-700"
            />
            <button onClick={handleSimulate} id='simulate' className="ml-0 bg-blue-500 rounded-xl p-3">
                Simulate &rarr;
            </button>
            {/* <Link onClick={handleSimulate} to='/Graph' id='simulate' className="bg-yellow-500 ml-3 rounded-xl p-3">
            </Link> */}
          </div>
    </div>
  )
}

export default InputData
