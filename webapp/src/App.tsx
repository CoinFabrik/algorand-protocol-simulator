import InputData from './pages/InputData'
import {
  BrowserRouter, Route, Routes,
} from 'react-router-dom'
import Graph from './pages/Graph'

function App (): JSX.Element {
  return (
    <BrowserRouter>
      <Routes>
        <Route path="/Graph" element={<Graph />} />
        <Route path="/" element={<InputData />} />
      </Routes>
    </BrowserRouter>
  )
}

export default App
