import { useState } from 'react'
// import { nodeInfo } from './NodeInfo'

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
        simulateVRF: 0,
    })

    // Handle input data
    const handleData = (e: React.ChangeEvent<HTMLInputElement>) => {
        setData({
            ...data,
            [e.target.name]: e.target.value
        })
        console.log(data)
    }

    const handleSimulate = () => {

        // address de cuenta - balance - status - nodo
        var nodeInfo = {
            "nodes": [{}],
            "links": [{}]
        }

        //create structure
        if(data.participationNodes > 0) {
            for (let i = 0; i < data.relayNodes; i++) {
                nodeInfo["nodes"].push({
                    "id": i,
                    "balance": data.balanceInAlgos/data.accountsPerNode,
                    "status": 1,
                    "node": i
                })
            }
            for (let i=0; i < data.relayNodes; i++) {
                nodeInfo["links"].push({
                    "source": i,
                    "target": i+1,
                    "value": Math.round(Math.random()*10+1)
                })
            }
        }

        const newNodeInfo = JSON.stringify(nodeInfo)
        const dataBlobBalances = new Blob([newNodeInfo], { type: 'application/json' });
        const blobURLBalances = URL.createObjectURL(dataBlobBalances);
        const downloadLinkBalances = document.createElement('a')
        downloadLinkBalances.href = blobURLBalances;
        downloadLinkBalances.download = "test_balance.json";
        downloadLinkBalances.click();
        URL.revokeObjectURL(blobURLBalances);

        // const dataBlobNetwork = new Blob([`${data.relayNodes} ${data.participationNodes} ${data.relayConnectionDelay} ${data.connectionDensity}`], { type: 'text/plain' });
        // const blobURLNetwork = URL.createObjectURL(dataBlobNetwork);
        // const downloadLinkNetwork = document.createElement('a')
        // downloadLinkNetwork.href = blobURLNetwork;
        // downloadLinkNetwork.download = "test_network.txt";
        // downloadLinkNetwork.click();
        // URL.revokeObjectURL(blobURLNetwork);

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
                <h2 className="text-center font-bold text-4xl">Consensous</h2>
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
        <div className="text-right mr-16 text-white">
        <a onClick={handleSimulate} id='simulate' className="bg-blue-500 rounded-xl p-3">
            Simulate &rarr;
        </a>
        </div>
    </div>
  )
}

export default InputData
