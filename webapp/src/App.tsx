import InputData from './pages/InputData'
import {
  BrowserRouter, Route, Routes
} from 'react-router-dom'
import Graph from './pages/Graph'

import { ContextProvider } from './context/Context'

function App (): JSX.Element {
  return (
    <ContextProvider>
      <BrowserRouter>
        <Routes>
          <Route path="/Graph" element={<Graph />} />
          <Route path="/" element={<InputData />} />
        </Routes>
      </BrowserRouter>
    </ContextProvider>
  )
}

export default App
