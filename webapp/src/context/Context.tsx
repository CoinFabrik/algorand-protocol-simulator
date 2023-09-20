import { type ReactNode, createContext, useState } from 'react'
import data from '../assets/data.json'

interface Props {
    children: ReactNode
}

const Context = createContext({
    context: data,
    setContext: (_context: any) => {}
})
  

export function ContextProvider ({ children }: Props): JSX.Element {
    const [context, setContext] = useState(data)
    return (
        <Context.Provider value={{ context, setContext }}>
            {children}
        </Context.Provider>
    )
}

export default Context
    