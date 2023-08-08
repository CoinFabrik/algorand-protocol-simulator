import { useState } from 'react'

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
        lambda: 0,
        lambda1: 0,
        lambda2: 0,
        omega: 0,
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
                />
                <h3 className="text-center text-medium mt-5 mb-1">Participation nodes</h3>
                <input
                    className="ml-16 text-center pl-2 p-1 w-2/3 border-2 border-slate-300 rounded-xl"
                    type='number'
                    placeholder='1456'
                />
                <h3 className="text-center text-medium mt-5 mb-1">Relay connection Delay (ms)</h3>
                <input
                    className="ml-16 text-center pl-2 p-1 w-2/3 border-2 border-slate-300 rounded-xl"
                    type='number'
                    placeholder='20'
                />
                <h3 className="text-center text-medium mt-5 mb-1">Conection Density (%)</h3>
                <input
                    className="ml-16 text-center pl-2 p-1 w-2/3 border-2 border-slate-300 rounded-xl"
                    type='number'
                    placeholder='48'
                />
            </div>
            <div className="w-1/4">
                <h2 className="text-center font-bold text-4xl">Accounts & Balances</h2>
                <h3 className="text-center text-medium mt-5 mb-1">Number of Accounts</h3>
                <input
                    className="ml-16 text-center pl-2 p-1 w-2/3 border-2 border-slate-300 rounded-xl"
                    type='number'
                    placeholder='159734'
                />
                <h3 className="text-center text-medium mt-5 mb-1">Total supply of Algos</h3>
                <input
                    className="ml-16 text-center pl-2 p-1 w-2/3 border-2 border-slate-300 rounded-xl"
                    type='number'
                    placeholder='201201201'
                />
                <h3 className="text-center text-medium mt-5 mb-1">Balance in Algos</h3>
                <input
                    className="ml-16 text-center pl-2 p-1 w-2/3 border-2 border-slate-300 rounded-xl"
                    type='number'
                    placeholder='1456'
                />
                <h3 className="text-center text-medium mt-5 mb-1">Accounts per node</h3>
                <input
                    className="ml-16 text-center pl-2 p-1 w-2/3 border-2 border-slate-300 rounded-xl"
                    type='number'
                    placeholder='10'
                />
                <h3 className="text-center text-medium mt-5 mb-1">Online Accounts (%)</h3>
                <input
                    className="ml-16 text-center pl-2 p-1 w-2/3 border-2 border-slate-300 rounded-xl"
                    type='number'
                    placeholder='23'
                />
            </div>
            <div className="w-1/4">
                <h2 className="text-center font-bold text-4xl">Consensous</h2>
                <h3 className="text-center text-medium mt-5 mb-1">Commites Values</h3>
                <input
                    className="ml-16 text-center pl-2 p-1 w-2/3 border-2 border-slate-300 rounded-xl"
                    type='number'
                    placeholder='7893'
                />
                <h3 className="text-center text-medium mt-5 mb-1">Seed Lookback</h3>
                <input
                    className="ml-16 text-center pl-2 p-1 w-2/3 border-2 border-slate-300 rounded-xl"
                    type='number'
                    placeholder='1456'
                />
                <h3 className="text-center text-medium mt-5 mb-1">Seed refresh interval (ms)</h3>
                <input
                    className="ml-16 text-center pl-2 p-1 w-2/3 border-2 border-slate-300 rounded-xl"
                    type='number'
                    placeholder='20'
                />
                <h3 className="text-center text-medium mt-5 mb-1">Balance Lookback</h3>
                <input
                    className="ml-16 text-center pl-2 p-1 w-2/3 border-2 border-slate-300 rounded-xl"
                    type='number'
                    placeholder='48'
                />
                <div className="flex flex-row ml-16 justify-center">
                    <div className="w-1/3">
                        <h3 className="text-medium mt-5 mb-1">λ (ms)</h3>
                        <input
                            className="text-center pl-2 p-1 w-2/3 border-2 border-slate-300 rounded-xl"
                            type='number'
                            placeholder='6'
                        />
                    </div>
                    <div className="w-1/3">
                        <h3 className="text-medium mt-5 mb-1">λ (ms)</h3>
                        <input
                            className="text-center pl-2 p-1 w-2/3 border-2 border-slate-300 rounded-xl"
                            type='number'
                            placeholder='23'
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
                        />
                    </div>
                    <div className="w-1/3">
                        <h3 className="text-medium mt-5 mb-1">𝚲 (ms)</h3>
                        <input
                            className="text-center pl-2 p-1 w-2/3 border-2 border-slate-300 rounded-xl"
                            type='number'
                            placeholder='4'
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
                />
                <h3 className="text-center text-medium mt-5 mb-1">Transaction pool limit</h3>
                <input
                    className="ml-16 text-center pl-2 p-1 w-2/3 border-2 border-slate-300 rounded-xl"
                    type='number'
                    placeholder='1456'
                />
                <h3 className="text-center text-medium mt-5 mb-1">Ledger Caché</h3>
                <input
                    className="ml-16 text-center pl-2 p-1 w-2/3 border-2 border-slate-300 rounded-xl"
                    type='number'
                    placeholder='20'
                />
                <h3 className="text-center text-medium mt-5 mb-1">Simulate VRF</h3>
                <input
                    className="ml-16 text-center pl-2 p-1 w-2/3 border-2 border-slate-300 rounded-xl"
                    type='bool'
                    placeholder='48'
                />
            </div>
        </div>
        <div className="text-right mr-16 text-white">
        <a href="/simulation" className="bg-blue-500 rounded-xl p-3">
            Simulate &rarr;
        </a>
        </div>
    </div>
  )
}

export default InputData
